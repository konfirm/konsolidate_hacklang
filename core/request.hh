<?hh

/**
 *  Provide easy access to all request data
 *  @name    CoreRequest
 *  @type    class
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreRequest<Konsolidate> extends Konsolidate
{
	protected string $_order;
	protected string $_raw;
	protected SimpleXMLElement $_xml;
	protected $_file;
	protected $_filereference;

	/**
	 *  magic __construct, CoreRequest constructor
	 *  @name    __construct
	 *  @type    constructor
	 *  @access  public
	 *  @param   object parent object
	 *  @return  object
	 *  @note    This object is constructed by one of Konsolidates modules
	 */
	function __construct($parent)
	{
		parent::__construct($parent);

		$this->_order         = $this->get('/Config/Request/variableorder', 'r');
		$this->_raw           = null;
		$this->_xml           = null;
		$this->_file          = null;
		$this->_filereference = null;
		$this->_collect();
	}

	/**
	 *  is the request a POST
	 *  @name    isPosted
	 *  @type    method
	 *  @access  public
	 *  @return  bool
	 */
	public function isPosted():bool
	{
		return $this->call('/Tool/isPosted');
	}

	/**
	 *  retrieve the raw request data
	 *  @name    getRawRequest
	 *  @type    method
	 *  @access  public
	 *  @return  string (bool false, if no raw data is available)
	 */
	public function getRawRequest():mixed
	{
		return !is_null($this->_raw) ? $this->_raw : false;
	}

	/**
	 *  retrieve XML request data
	 *  @name    getXML
	 *  @type    method
	 *  @access  public
	 *  @return  SimpleXMLElement (bool false, if no xml data is available)
	 *  @syntax  SimpleXMLElement CoreRequest->getXML()
	 */
	public function getXML():mixed
	{
		return !is_null($this->_xml) ? $this->_xml : false;
	}

	/**
	 *  retrieve variables from a raw data request
	 *  @name    _collectFromRaw
	 *  @type    method
	 *  @access  protected
	 *  @return  void
	 */
	protected function _collectFromRaw():void
	{
		$this->_raw = trim(file_get_contents('php://input'));  //  Try to determine what kind of request triggered this class
		switch (substr($this->_raw, 0, 1))
		{
			case '<':  //  XML  //  in-class for now
				$this->_xml = new SimpleXMLElement($this->_raw);

				foreach ($this->_xml as $key=>$value)
					$this->$key = (string) $value;
				break;
		}
	}

	/**
	 *  retrieve variables from a HTTP request (POST/GET only)
	 *  @name    _collectHTTP
	 *  @type    method
	 *  @access  protected
	 *  @return  bool
	 */
	protected function _collectHTTP(array<string, mixed> $collection):bool
	{
		if (is_array($collection) && (bool) count($collection))
		{
			foreach ($collection as $param=>$value)
				$this->$param = $value;

			return true;
		}

		return false;
	}

	/**
	 *  retrieve variables and assign them to the Request module
	 *  @name    _collect
	 *  @type    method
	 *  @access  protected
	 *  @return  void
	 */
	protected function _collect():void
	{  //  gather variables and if request method is post and it failed to gather variables, try to collect data from raw input.
		if (!$this->_collectHTTP($this->_getCollection()) && $this->isPosted())
			$this->_collectFromRaw();  //  if the request method is post and the appear to be (one or more) files attached, prepare those aswel
		if ($this->isPosted() && is_array($_FILES) && count($_FILES))
			$this->_collectFiles();
	}


	/**
	 *  Determine the proper variable processing order
	 *  @name    _getCollection
	 *  @type    method
	 *  @access  protected
	 *  @return  array     if no order is specified, _GET or _POST global, merged result of the desired order otherwise
	 *  @note    By default _getCollection module will distinguish between GET and POST requests, they will not be processed both!
	 *           You can override this behaviour by setting the variable order (EGPCS, like the variables_order php.ini setting) to /Config/Request/variableorder
	 *           E.g. $this->set('/Config/Request/variableorder', 'GP');  //  combine GET and POST variables
	 */
	protected function _getCollection():array<string, mixed>
	{
		if (!is_null($this->_order))
		{
			$return = Array();
			for ($i = 0; $i < strlen($this->_order); ++$i)
				switch (strToUpper($this->_order{$i}))
				{
					case 'G':
						$return = array_merge($return, $_GET);
						break;

					case 'P':
						$return = array_merge($return, $_POST);
						break;

					case 'C':
						$return = array_merge($return, $_COOKIE);
						break;

					case 'R':
						$return = array_merge($return, $_REQUEST);
						 break;

					case 'E':
						$return = array_merge($return, $_ENV);
						break;

					case 'S':
						$return = array_merge($return, $_SERVER);
						break;

				}

			return $return;
		}

		return $this->isPosted() ? $_POST : $_GET;
	}

	/**
	 *  Gather file information attached to the (POST only) request
	 *  @name    _collectFiles
	 *  @type    method
	 *  @access  protected
	 *  @return  void
	 */
	protected function _collectFiles():void
	{
		$this->_file          = Array();
		$this->_filereference = Array();

		foreach ($_FILES as $field=>$file)
			if (isset($file['error']))  //  we have one or more file
			{
				if (is_array($file['error']))  //  multiple files
				{
					$fileList = Array();
					foreach ($file['error'] as $key=>$void)
					{
						$file = $this->_createFileInstance($file, $field, $key);
						array_push($this->_file, $file);
						array_push($fileList, $file);
					}
				}
				else  //  single file
				{
					$fileList = $this->_createFileInstance($file, $field);
					array_push($this->_file, $fileList);
				}

				$this->_filereference[$field] = $fileList;
			}
	}

	/**
	 *  Create and populate an (unique) instance of the Request/File module
	 *  @name    _createFileInstance
	 *  @type    method
	 *  @access  protected
	 *  @param   array   _FILES record
	 *  @param   string  fieldname
	 *  @param   string  reference (only when multiple files are uploaded)
	 *  @return  *RequestFile instance
	 */
	protected function _createFileInstance(array<string, string> $file, string $variable, ?string $reference):CoreRequestFile
	{
		$tmp = $this->instance('File');
		$tmp->variable = $variable;

		foreach ($file as $key=>$value)
			$tmp->{$key} = is_null($reference) ? $value : $value[$reference];

		return $tmp;
	}

	/**
	 *  Does the request have files attached?
	 *  @name    hasFiles
	 *  @type    method
	 *  @access  public
	 *  @return  bool
	 */
	public function hasFiles():bool
	{
		return is_array($this->_file) && (bool) count($this->_file);
	}


	/**
	 *  Obtain the array of files
	 *  @name    getFiles
	 *  @type    method
	 *  @access  public
	 *  @param   bool  referenced array (non referenced array containing variable names)
	 *  @return  array files
	 */
	public function getFiles(bool $reference=false):array<string, array>
	{
		return $reference ? $this->_filereference : $this->_file;
	}


	/**
	 *  Obtain a specific filereference (formfield)
	 *  @name    getFileByReference
	 *  @type    method
	 *  @access  public
	 *  @param   string reference name
	 *  @return  mixed  file object, array of file objects or false if reference does not exist
	 */
	public function getFileByReference(string $reference):mixed
	{
		return isset($this->_filereference[$reference]) ? $this->_filereference[$reference] : false;
	}
}