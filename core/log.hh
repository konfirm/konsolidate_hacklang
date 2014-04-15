<?hh  //  strict

/**
 *  Basic verbosity based logging
 *  @name    CoreLog
 *  @type    class
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreLog extends Konsolidate
{
	/**
	 *  The verbositylevel you wish to log
	 *  0 (Critical), 1 (Severe), 2 (Warning), 3 (Info), 4 (Debug)
	 *  @name    _verbositylevel
	 *  @type    int
	 *  @access  protected
	 */
	protected int $_verbositylevel;

	/**
	 *  The logfile to which to write the log data
	 *  @name    _logfile
	 *  @type    string
	 *  @access  protected
	 */
	protected string $_logfile;


	/**
	 *  constructor
	 *  @name    __construct
	 *  @type    constructor
	 *  @access  public
	 *  @param   object parent object
	 *  @return  object
	 *  @note    This object is constructed by one of Konsolidates modules
	 */
	public function __construct(Konsolidate $parent)
	{
		parent::__construct($parent);

		$this->_logfile = $this->get('/Config/konsolidate/log', ini_get('error_log'));
		$this->setVerbosity($this->get('/Config/konsolidate/loglevel'));
	}

	/**
	 *  set the verbosity level of Konsolidate
	 *  @name    setVerbosity
	 *  @type    method
	 *  @access  public
	 *  @param   int level (default matching error_reporting ini directive)
	 *  @return  void
	 */
	public function setVerbosity(?int $level):void
	{
		if (is_null($level))
			$level = $this->_determineVerbosity();
		$this->_verbositylevel = $level;
	}

	/**
	 *  Output messages according to preferences in the php.ini (override in Konsolidate Config)
	 *  @name    message
	 *  @type    method
	 *  @access  public
	 *  @param   string message
	 *  @param   int    level
	 *  @return  void
	 *  @note    Configuration options: display_errors (Config/Log/displayerrors), log_errors (Config/Log/logerrors)
	 */
	public function message(string $message, int $verbosity=3):void
	{
		if ($verbosity <= $this->_verbositylevel)
		{
			if ((bool) ini_get('display_errors'))
			{
				if ((bool) ini_get('html_errors'))
					print $this->_formatMessage($message, $verbosity, true);
				else
					print $this->_formatMessage($message, $verbosity) . PHP_EOL;
			}

			if ((bool) ini_get('log_errors'))
				$this->write($message, $verbosity);
		}
	}

	/**
	 *  write a (formatted) line to the log file
	 *  @name    write
	 *  @type    method
	 *  @access  public
	 *  @param   string message
	 *  @param   int    level
	 *  @return  bool
	 *  @note    if there's any reason the message cannot be written to the logfile, the message is written into the default error.log
	 */
	public function write(string $message, int $verbosity=3):bool
	{
		if ($verbosity <= $this->_verbositylevel)
		{
			if (!error_log($this->_formatMessage($message, $verbosity ) . PHP_EOL, 0, $this->_logfile))
			{
				error_log($message);
				return false;
			}
			return true;
		}

		return false;
	}

	/**
	 *  Translate the verbosity level int to a more readable string
	 *  @name    _translate
	 *  @type    method
	 *  @access  protected
	 *  @param   int    level
	 *  @param   bool   uppercase (default true)
	 *  @return  bool
	 *  @syntax  bool CoreLog->_translate( int level [, bool uppercase ] )
	 */
	protected function _translate(int $verbosity, bool $upperCase=true):string
	{
		$return = 'Unknown';

		switch( (int) $verbosity )
		{
	 		case 0:
	 			$return = 'Critical'; 
	 			break;

	 		case 1:
	 			$return = 'Severe';   
	 			break;

	 		case 2:
	 			$return = 'Warning';  
	 			break;

	 		case 3:
	 			$return = 'Info';     
	 			break;

	 		case 4:
	 			$return = 'Debug';    
	 			break;

		}

		if ($upperCase)
			$return = strToUpper($return);

		return $return;
	}

	/**
	 *  Determine the verbosity level based on the error_reporting directive
	 *  @name    _determineVerbosity
	 *  @type    method
	 *  @access  protected
	 *  @return  int  level
	 */
	protected function _determineVerbosity():int
	{
		$reporting = ini_get('error_reporting');
		$level     = 3;

		if (E_STRICT & $reporting)
			$level = 4; //  Debug - Very strict, al lot of information
		else if (E_ALL & $reporting)
			$level = 3; //  All - A lot of information
		else if (E_NOTICE & $reporting)
			$level = 2; //  Info - Much information
		else if (E_WARNING & $reporting)
			$level = 1; // Only warnings - less informative
		else if (E_ERROR & $reporting)
			$level = 0; // Only critial information - least informative

		return $level;
	}

	/**
	 *  Format the log message
	 *  @name    _formatMessage
	 *  @type    method
	 *  @access  protected
	 *  @param   string message
	 *  @param   int    level
	 *  @param   bool   html
	 *  @return  string message
	 */
	protected function _formatMessage(string $message, int $verbosity, bool $html=false):string
	{
		if ($html)
			return '<div class="konsolidate_error konsolidate_' . strToLower($this->_translate($verbosity)) . '"><span class="konsolidate_time">' . date('r') . '</span> <span class="konsolidate_level">' . $this->_translate($verbosity) . '</span> <span class="konsolidate_script">' . $_SERVER['SCRIPT_NAME'] . '</span> <span class="konsolidate_message">' . $message . '</span></div>';
		return '[' . $this->_translate($verbosity) . '] - ' . $_SERVER['SCRIPT_NAME'] . ']: ' . $message;
	}
}
