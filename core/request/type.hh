<?hh  //  strict

/**
 *  Provide easy access to all request data for a specific request method
 *  @name    CoreRequest
 *  @type    class
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreRequestType<Konsolidate> extends Konsolidate implements ArrayAccess
{
	protected $_protect;
	protected $_type;

	/**
	 *  __construct, CoreRequestType constructor
	 *  @name    __construct
	 *  @type    constructor
	 *  @access  public
	 *  @param   object parent object
	 *  @param   string type [optional, default null - the request method used]
	 *  @return  object
	 */
	public function __construct(Konsolidate $parent, ?string $type)
	{
		parent::__construct($parent);

		$this->_type    = strToUpper(!empty($type) ? $type : $_SERVER['REQUEST_METHOD']);
		$this->_protect = $this->get('/Config/Request/protect_' . strToLower($this->_type), $this->get('/Config/Request/protect', true));
		$this->_collect();
	}

	/**
	 *  Catch custom property setter calls early on to intercept in case the request type is protected from tampering
	 *  @name    __set
	 *  @type    magic function
	 *  @access  public
	 *  @param   string name
	 *  @param   mixed  value
	 *  @return  void
	 */
	public function __set(string $name, mixed $value):void
	{
		if ($this->_protect)
			return $this->call('/Log/message', __METHOD__ . ' not allowed to modify ' . strToUpper($this->_type) . ' request variables', 2);

		return parent::__set($name, $value);
	}

	/**
	 *  Collect the variables in the current buffer and overwrite the superglobals in such fashion that developers won't notice
	 *  they are talking to our module instead of the usual superglobals ($_GET and $_POST are replace by an instance of this class, 
	 *  which allow for array-like behaviour due to the ArrayAccess implementation)
	 *  @name    _collect
	 *  @type    method
	 *  @access  protected
	 *  @return  void
	 */
	protected function _collect():void
	{
		//  determine the collection and try to populate it's properties
		switch ($this->_type)
		{
			//  use PHP's built-in _GET and/or _POST superglobals, override after copying
			case 'GET':
			case 'POST':
				$super = '_' . $this->_type;
				if (isset($GLOBALS[$super]) && is_array($GLOBALS[$super]))
				{
					$this->_property = array_merge($this->_property, $GLOBALS[$super]);
				}
				$GLOBALS[$super] = $this;
				break;

			//  provide PUT and DELETE support
			case 'PUT':
			case 'DELETE':
				$super = '_' . $this->_type;
				if (isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] == strToUpper($this->_type))
				{
					$raw = trim(file_get_contents("php://input"));
					if (!empty($raw))
						parse_str($raw, $this->_property);
				}
				$GLOBALS[$super] = $this;
				break;

			default:
				$this->call('/Log/message', 'Unsupported request type: ' . $this->_type, 1);
				break;
		}
	}


	/*  ArrayAccess implementation */
	public function offsetGet($offset):mixed
	{
		return $this->$offset;
	}

	public function offsetSet($offset, $value):void
	{
		$this->$offset = $value;
	}

	public function offsetExists($offset):bool
	{
		return isset($this->$offset);
	}

	public function offsetUnset($offset):void
	{
		unset($this->$offset);
	}
}
