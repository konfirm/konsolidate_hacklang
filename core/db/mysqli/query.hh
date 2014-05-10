<?hh  //  strict


/**
 *  MySQLi result set (this object is instanced and returned for every query)
 *  @name    CoreDBMySQLiQuery
 *  @type    class
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreDBMySQLiQuery<Konsolidate> extends Konsolidate
{
	/**
	 *  The connection resource
	 *  @name    _conn
	 *  @type    resource
	 *  @access  protected
	 */
	protected resource $_conn;

	/**
	 *  The result resource
	 *  @name    _result
	 *  @type    resource
	 *  @access  protected
	 */
	protected resource $_result;

	/**
	 *  The query
	 *  @name    query
	 *  @type    string
	 *  @access  public
	 */
	public string $query;

	/**
	 *  The exception object, used to populate 'error' and 'errno' properties
	 *  @name    exception
	 *  @type    object
	 *  @access  public
	 */
	public CoreDBMySQLiException $exception;

	/**
	 *  The error message
	 *  @name    error
	 *  @type    string
	 *  @access  public
	 */
	public string $error;

	/**
	 *  The error number
	 *  @name    errno
	 *  @type    int
	 *  @access  public
	 */
	public int $errno;

	/**
	 *  execute given query on given connection
	 *  @name    execute
	 *  @type    method
	 *  @access  public
	 *  @param   string   query
	 *  @param   MySQLi connection
	 *  @returns void
	 */
	public function execute(string $query, MySQLi $connection):void
	{
		$this->query    = $query;
		$this->_conn    = $connection;
		$start          = microtime(true);
		$this->_result  = $this->_conn->query($this->query);
		$this->duration = microtime(true) - $start;

		if ($this->_result instanceof MySQLi_Result)
			$this->rows = $this->_result->num_rows;
		else if ($this->_result === true)
			$this->rows = $this->_conn->affected_rows;

		//  We want the exception object to tell us everything is going extremely well, don't throw it!
		$this->import('../exception.hh');
		$this->exception = new CoreDBMySQLiException($this->_conn);
		$this->errno     = &$this->exception->errno;
		$this->error     = &$this->exception->error;

		if ($this->errno > 0)
			$this->call('/Log/write', __METHOD__ . ' failed: (' . $this->errno . ') ' . $this->error . PHP_EOL . '--> ' . $this->query, 2);
	}

	/**
	 *  rewind the internal resultset
	 *  @name    rewind
	 *  @type    method
	 *  @access  public
	 *  @returns bool success
	 */
	public function rewind():bool
	{
		if ($this->_result instanceof MySQLi_Result && $this->_result->num_rows > 0)
			return $this->_result->data_seek(0);

		return false;
	}

	/**
	 *  get the next result from the internal resultset
	 *  @name    next
	 *  @type    method
	 *  @access  public
	 *  @returns object resultrow
	 */
	public function next():mixed
	{
		if ($this->_result instanceof MySQLi_Result)
			return $this->_result->fetch_object();

		return false;
	}

	/**
	 *  get the ID of the last inserted record
	 *  @name    lastInsertID
	 *  @type    method
	 *  @access  public
	 *  @returns int id
	 */
	public function lastInsertID():int
	{
		return $this->_conn->insert_id;
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
	public function lastId():int
	{
		return $this->lastInsertID();
	}

	/**
	 *  Retrieve an array containing all resultrows as objects
	 *  @name    fetchAll
	 *  @type    method
	 *  @access  public
	 *  @returns array result
	 */
	public function fetchAll():array
	{
		$return = Array();
		while( $oRecord = $this->next() )
			array_push( $return, $oRecord );
		$this->rewind();
		return $return;
	}

	public function __destruct()
	{
		if (is_resource( $this->_result))
			mysqli_free_result($this->_result);
	}
}
