<?hh  //  strict


/**
 *  MySQLi specific Exception class
 *  @name    CoreDBMySQLiException
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreDBMySQLiException<Exception> extends Exception
{
	/**
	 *  The error message
	 *  @name    error
	 *  @type    string
	 *  @access  public
	 */
	public string $error;

	/**
	 *  The error number
	 *  @name    error
	 *  @type    int
	 *  @access  public
	 */
	public int $errno;

	/**
	 *  constructor
	 *  @name    __construct
	 *  @type    constructor
	 *  @access  public
	 *  @param   resource connection
	 *  @returns object
	 *  @syntax  object = &new CoreDBMySQLiException( resource connection )
	 *  @note    This object is constructed by CoreDBMySQLi as 'status report'
	 */
	public function __construct()
	{
		$arg = func_get_args();
		if (count($arg) === 2)
		{
			$this->error = $arg[0];
			$this->errno = $arg[1];
		}
		else
		{
			$connection = array_shift( $arg );
			$this->error = $connection->error;
			$this->errno = $connection->errno;
		}
	}
}
