<?hh  //  strict


/**
 *  MySQL Connectivity
 *  @name    CoreDBMySQLi
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreDBMySQLi<Konsolidate> extends Konsolidate
{
	/**
	 *  The connection URI (parsed url)
	 *  @name    _URI
	 *  @type    Map
	 *  @access  protected
	 */
	protected Map<string, string> $_URI;

	/**
	 *  The connection resource
	 *  @name    _conn
	 *  @type    resource
	 *  @access  protected
	 */
	protected resource $_conn;

	/**
	 *  The query cache
	 *  @name    _cache
	 *  @type    array
	 *  @access  protected
	 */
	protected Map<string, resource> $_cache;

	/**
	 *  Wether or not a transaction is going on
	 *  @name    _transaction
	 *  @type    bool
	 *  @access  protected
	 */
	protected bool $_transaction;

	/**
	 *  The error object (Exception which isn't thrown)
	 *  @name    error
	 *  @type    object
	 *  @access  public
	 */
	public MySQLiException $error;

	/**
	 *  Replacements for fingerprinting
	 *  @name    _fingerprintreplacement
	 *  @type    object
	 *  @access  protected
	 */
	protected Map<string, string> $_fingerprintreplacement;


	/**
	 *  constructor
	 *  @name    __construct
	 *  @type    constructor
	 *  @access  public
	 *  @param   object parent object
	 *  @returns object
	 *  @note    This object is constructed by one of Konsolidates modules
	 */
	public function __construct(Konsolidate $parent)
	{
		parent::__construct($parent);

		$this->_URI         = null;
		$this->_conn        = null;
		$this->_cache       = Map<string, resource> {};
		$this->error        = null;
		$this->_transaction = false;

		$this->_fingerprintreplacement = Map<string, string> {
			'string' => $this->get('/Config/MySQL/fingerprint_string', '"$"'),
			'number' => $this->get('/Config/MySQL/fingerprint_number', '#'),
			'NULL'   => $this->get('/Config/MySQL/fingerprint_null', 'NULL'),
			'names'  => $this->get('/Config/MySQL/fingerprint_names', '`?`')
		};
	}

	/**
	 *  Assign the connection DSN
	 *  @name    setConnection
	 *  @type    method
	 *  @access  public
	 *  @param   string DSN URI
	 *  @returns bool
	 */
	public function setConnection(string $uri):bool
	{
		$this->_URI = Map::fromArray(parse_url($uri));

		if (!$this->_URI->contains('host'))
			$this->exception('Missing required host from the MySQLi DSN "' . $uri . '"');
        else if (!$this->_URI->contains('user'))
			$this->exception('Missing required username from the MySQLi DSN "' . $uri . '""');

		return true;
	}

	/**
	 *  Connect to the database
	 *  @name    connect
	 *  @type    method
	 *  @access  public
	 *  @returns bool
	 *  @note    An explicit call to this method is not required, since the query method will create the connection if
     *           it isn't connected
	 */
	public function connect():bool
	{
		if (!$this->isConnected())
		{
			$this->_conn = new MySQLi(
                $this->_URI->get('host'),
                $this->_URI->get('user'),
                $this->_URI->get('pass') ?: '',
                trim($this->_URI->get('path'), '/'),
                $this->_URI->get('port') ?: 3306
			);

			if ($this->_conn->connect_error)
				$this->exception($this->_conn->connect_error, $this->_conn->connect_errno);
		}

		return true;
	}

	/**
	 *  Disconnect from the database
	 *  @name    disconnect
	 *  @type    method
	 *  @access  public
	 *  @returns bool
	 */
	public function disconnect():bool
	{
		if ( $this->isConnected() )
			return $this->_conn->close();

		return true;
	}

	/**
	 *  Check to see whether a connection is established
	 *  @name    isConnected
	 *  @type    method
	 *  @access  public
	 *  @returns bool
	 */
	public function isConnected():bool
	{
		return is_object($this->_conn);
	}

	/**
	 *  Query the database
	 *  @name    query
	 *  @type    method
	 *  @access  public
	 *  @param   string query
	 *  @paran   bool   usecache (default true)
	 *  @paran   bool   add info (default false)
	 *  @paran   bool   extended info (default true)
	 *  @returns object result
	 *  @note    Requesting extended information will automatically enable normal info aswel
	 */
	public function query(string $query, bool $cache=true, bool $info=false, bool $extendedInfo=false):?CoreDBMySQLiQuery
	{
		$cacheKey = md5($query);
		if ($cache && $this->_cache->contains($cacheKey))
		{
            $result = $this->_cache->get($cacheKey);
			$result->rewind();
			$result->cached = true;
			return $result;
		}

		if ($this->connect())
		{
			$result = $this->instance('Query');
			$result->execute($query, $this->_conn);
			$result->info   = $info || $extendedInfo ? $this->info($extendedInfo, Array('duration' => $result->duration)) : 'additional query info not processed';
			$result->cached = false;

			if ($cache && $this->_isCachableQuery($query))
				$this->_cache->set($cacheKey, $result);

			return $result;
		}

		return null;
	}

	/**
	 *  create a fingerprint for given query, attempting to remove all variable components
	 *  @name    fingerprint
	 *  @type    method
	 *  @access  public
	 *  @param   string   query
	 *  @param   bool     hash output (default true)
	 *  @param   bool     strip escaped names (default false)
	 *  @returns string   fingerprint
	 */
	public function fingerprint(string $query, bool $hash=true, bool $stripName=false, Map<string, string> $replace=Map {}):string
	{
		$replace = array_merge($this->_fingerprintreplacement, $replace);
		$string  = $replace->get('string');
		$number  = $replace->get('number');
		$null    = $replace->get('NULL');
		$names   = $replace->get('names');
		$replace = Array(
			'/([\"\'])(?:.*[^\\\\]+)*(?:(?:\\\\{2})*)+\1/xU' => $string, //  replace quoted variables
			"/--.*?[\r\n]+/"=>"",                                        //  strip '--' comments
			"/#.*?[\r\n]+/"=>"",                                         //  strip '#' comments
			"/\/\*.*?\*\//"=>"",                                         //  strip /* */ comments
			"/\s*([\(\)=\+\/,-]+)/"=>"\\1",                              //  strip whitespace left of specific chars
			"/([\(=\+\/,-]+)\s*/"=>"\\1",                                //  strip whitespace right of specific chars
			"/\b[0-9]*[\.]*[0-9]+\b/" => $number,                        //  replace numbers which appear to be values
			"/\bNULL\b/i" => $null,                                      //  replace NULL values
			"/\s+/"=>" "                                                 //  replace (multiple) whitespace chars by space
		);

		if ( $stripName )
			$replace->set('/`.*?`/', $names);
		$result = trim(preg_replace($replace->keys(), $replace->values(), $query));

		return $hash ? md5($result) : $result;
	}

	/**
	 *  get the ID of the last inserted record
	 *  @name    lastInsertID
	 *  @type    method
	 *  @access  public
	 *  @returns int id
	 */
	public function lastInsertID():?int
	{
        return $this->isConnected() ? mysqli_insert_id($this->_conn) : null;
	}

	/**
	 *  get the ID of the last inserted record
	 *  @name    lastId
	 *  @type    method
	 *  @access  public
	 *  @returns int id
	 *  @note    alias for lastInsertID
	 *  @see     lastInsertID
	 */
	public function lastId():?int
	{
		return $this->lastInsertID();
	}

	/**
	 *  Properly escape a string
	 *  @name    escape
	 *  @type    method
	 *  @access  public
	 *  @param   string input
	 *  @returns string escaped input
	 */
	public function escape(string $string):string
	{
		if ($this->connect())
			return mysqli_real_escape_string($this->_conn, $string);

		$this->call('/Log/write', get_class($this) . '::escape, could not escape string');

		return false;
	}

	/**
	 *  Quote and escape a string
	 *  @name    quote
	 *  @type    method
	 *  @access  public
	 *  @param   string input
	 *  @returns string quoted escaped input
	 */
	public function quote(string $string):string
	{
		return '"' . $this->escape($string) . '"';
	}

	/**
	 *  Start transaction
	 *  @name    startTransaction
	 *  @type    method
	 *  @access  public
	 *  @returns bool success
	 */
	public function startTransaction():bool
	{
		if ($this->connect() && !$this->_transaction)
			$this->_transaction = $this->_conn->autocommit(false);

		return $this->_transaction;
	}

	/**
	 *  End transaction by sending 'COMMIT' or 'ROLLBACK'
	 *  @name    startTransaction
	 *  @type    method
	 *  @access  public
	 *  @param   bool commit [optional, default true]
	 *  @returns bool success
	 *  @note    if argument 'commit' is true, 'COMMIT' is sent, 'ROLLBACK' otherwise
	 */
	public function endTransaction($success=true):bool
	{
		if ($this->_transaction)
			$this->_transaction = !($success ? $this->_conn->commit() : $this->_conn->rollback());

		return !$this->_transaction;
	}

	/**
	 *  Commit a transaction
	 *  @name    commitTransaction
	 *  @type    method
	 *  @access  public
	 *  @returns bool success
	 *  @note    same as endTransaction( true );
	 */
	public function commitTransaction():bool
	{
		return $this->endTransaction(true);
	}

	/**
	 *  Rollback a transaction
	 *  @name    rollbackTransaction
	 *  @type    method
	 *  @access  public
	 *  @returns bool success
	 *  @note    same as endTransaction( false );
	 */
	public function rollbackTransaction():bool
	{
		return $this->endTransaction(false);
	}


	/**
	 *  Returns the default character set for the database connection
	 *  @name    characterSetName
	 *  @type    method
	 *  @access  public
	 *  @returns string characterset
	 */
	public function characterSetName():string
	{
		return $this->_conn->character_set_name();
	}

	/**
	 *  Returns the MySQLi client version
	 *  @name    clientVersion
	 *  @type    method
	 *  @access  public
	 *  @param   bool  versionstring [optional, default false]
	 *  @returns mixed version
	 */
	public function clientVersion(bool $versionString=false):mixed
	{
		if ($versionString)
			return $this->_versionToString($this->_conn->client_version);

		return $this->_conn->client_version;
	}

	/**
	 *  Returns the MySQLi client info
	 *  @name    clientInfo
	 *  @type    method
	 *  @access  public
	 *  @returns string info
	 *  @note    the client info may appear to be the version as string, but can contain additional build information,
     *           use clientVersion( true ) for fool-proof string version comparing
	 */
	public function clientInfo():string
	{
		return $this->_conn->client_info;
	}

	/**
	 *  Returns the MySQLi server version
	 *  @name    serverVersion
	 *  @type    method
	 *  @access  public
	 *  @param   bool  versionstring [optional, default false]
	 *  @returns mixed version (false if a connection could not be established)
	 */
	public function serverVersion($versionString=false):mixed
	{
		if (!$this->connect())
			return false;

		if ($versionString)
			return $this->_versionToString($this->_conn->server_version);

		return $this->_conn->server_version;
	}

	/**
	 *  Returns the MySQLi server info
	 *  @name    serverInfo
	 *  @type    method
	 *  @access  public
	 *  @returns string info
	 *  @note    the server info may appear to be the version as string, but can contain additional build information,
     *           use serverVersion( true ) for fool-proof string version comparing
	 */
	public function serverInfo():string
	{
		return $this->_conn->server_info;
	}

	/**
	 *  Retrieves information about the most recently executed query
	 *  @name    info
	 *  @type    method
	 *  @access  public
	 *  @param   bool extendedinfo [optional, default false]
	 *  @returns object info
	 *  @note    by requesting extended info, the connection stats are added to the info object
	 */
	public function info(bool $extendInfo=false, Array $appendInfo=null):CoreDBMySQLiInfo
	{
		$result = $this->instance('Info');
		$result->process($this->_conn, $extendInfo, $appendInfo);

		return $result;
	}

	/**
	 *  Convert the given version integer back to its string representation
	 *  @name    _versionToString
	 *  @type    method
	 *  @access  protected
	 *  @param   int    version
	 *  @returns string version
	 */
	protected function _versionToString(int $version):string
	{
		$major = round($version / 10000);
		$minor = round(($version - ($major * 10000)) / 100);

		return sprintf('%d.%d.%d', $major, $minor, ($version - (($major * 10000) + ($minor * 100))));
	}

	/**
	 *  Determine whether a query should be cached (this applies only to 'SELECT' queries)
	 *  @name    _isCachableQuery
	 *  @type    method
	 *  @access  protected
	 *  @param   string query
	 *  @returns bool   success
	 */
	protected function _isCachableQuery($query):bool
	{
		return (bool) preg_match('/^\s*SELECT /i', $query);
	}
}
