<?hh  //  strict


/**
 *  Network socket connectivity
 *  @name    CoreNetworkSocket
 *  @type    class
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreNetworkSocket<Konsolidate> extends Konsolidate {
	protected resource $_conn;
	protected int $_timeout;

	public int $errno;
	public string $error;

	/**
	 *  CoreNetworkSocket constructor
	 *  @name    __construct
	 *  @type    constructor
	 *  @access  public
	 *  @param   object parent object
	 *  @return  object
	 *  @note    This object is constructed by one of Konsolidates modules
	 */
	public function __construct(Konsolidate $parent) {
		parent::__construct($parent);

		$this->_timeout = 10;
	}

	/**
	 *  Connect to given host, port using the provided transport
	 *  @name    connect
	 *  @type    method
	 *  @access  public
	 *  @param   string  host
	 *  @param   int     port
	 *  @param   string  transport [optional, default 'tcp']
	 *  @param   int     timeout [optional, default 10]
	 *  @return  bool    success
	 */
	public function connect(string $host, int $port, string $transport='tcp', int $timeout=null):bool {
		//  ipv6 addresses ought to be wrapped in square brackets, it appears we can safely assume ipv6 styles
		//  addresses always have 2+ colons.
		if (substr_count($host, ':') > 1) {
			$host = '[' . $host . ']';
		}

		$dsn = trim(sprintf('%s://%s:%d', $transport, $host, $port), ':');
		$this->_conn = stream_socket_client($dsn, $errno, $errstr, $this->timeout($timeout));

		if (!$this->_conn) {
			$this->errno = $errno;
			$this->error = $errstr;
		}
		else if (is_resource($this->_conn)) {
			$this->timeout($timeout);

			return true;
		}

		return false;
	}

	/**
	 *  Disconnect a connection
	 *  @name    disconnect
	 *  @type    method
	 *  @access  public
	 *  @return  bool  success
	 */
	public function disconnect():bool {
		if (is_resource($this->_conn)) {
			return fclose($this->_conn);
		}

		return false;
	}

	/**
	 *  Get/set the timeout for a connection (if a connection is already established and the timeout could not
	 *  be set, bool false is returned).
	 *  @name    timeout
	 *  @type    method
	 *  @access  public
	 *  @param   int    timeout [optional, default null - return the current timeout]
	 *  @return  mixed  timeout
	 */
	public function timeout(int $timeout=null):mixed {
		if (!is_null($timeout)) {
			if (is_resource($this->_conn) && !stream_set_timeout($this->_conn, $timeout)) {
				return false;
			}

			$this->_timeout = $timeout;
		}

		return $this->_timeout;
	}

	/**
	 *  Write data to the established connection
	 *  @name    write
	 *  @type    method
	 *  @access  public
	 *  @param   string  data
	 *  @return  bool    success
	 *  @note    if no connection is established bool false will be returned
	 */
	public function write(string $data):bool {
		if (is_resource( $this->_conn)) {
			return stream_socket_sendto($this->_conn, $data) >= 0;
		}

		return false;
	}

	/**
	 *  Read data from the established connection
	 *  @name    read
	 *  @type    method
	 *  @access  public
	 *  @param   int    length [optional, default 512]
	 *  @return  mixed  result [one of: string data or bool success]
	 */
	public function read(int $length=512) {
		if (is_resource($this->_conn)) {
			return stream_socket_recvfrom($this->_conn, $length);
		}

		return false;
	}
}
