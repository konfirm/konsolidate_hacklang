<?hh  //  strict


/**
 *  Provide easy access to all request data
 *  @name    CoreRequest
 *  @type    class
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreRequest<Konsolidate> extends Konsolidate
{
	/**
	 *  CoreRequest constructor
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

		$this->_collect();
	}

	/**
	 *  Was the request of type GET
	 *  @name    isGet
	 *  @type    method
	 *  @access  public
	 *  @return  bool
	 */
	public function isGet():bool
	{
		return isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] === 'GET';
	}

	/**
	 *  Was the request of type POST
	 *  @name    isPost
	 *  @type    method
	 *  @access  public
	 *  @return  bool
	 */
	public function isPost():bool
	{
		return isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] === 'POST';
	}

	/**
	 *  Was the request of type PUT
	 *  @name    isPut
	 *  @type    method
	 *  @access  public
	 *  @returns bool
	 */
	public function isPut():bool
	{
		return isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] === 'PUT';
	}

	/**
	 *  Was the request of type DELETE
	 *  @name    isDelete
	 *  @type    method
	 *  @access  public
	 *  @returns bool
	 */
	public function isDelete():bool
	{
		return isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] === 'DELETE';
	}


	/**
	 *  get a property value from a module using a path, in contrast to the overridden getter method
	 *  this implementation takes the various supported request methods into consideration as sub modules
	 *  @name    get
	 *  @type    method
	 *  @access  public
	 *  @param   string   path to the property to get
	 *  @param   mixed    default return value (optional, default null)
	 *  @return  mixed
	 *  @note    supplying a default value should be done per call, the default is never stored
	 */
	public function get():mixed
	{
		$arg       = func_get_args();
		$key       = array_shift($arg);
		$default   = count($arg) ? array_shift($arg) : null;
		$seperator = strrpos($key, static::MODULE_SEPARATOR);

		if ($seperator !== false && ($module = $this->getModule(substr($key, 0, $seperator))) !== false)
		{
			return $module->get(substr($key, $seperator + 1), $default);
		}
		else if ($this->{$_SERVER['REQUEST_METHOD']} && isset($this->{$_SERVER['REQUEST_METHOD']}->{$key}))
		{
			return $this->{$_SERVER['REQUEST_METHOD']}->{$key};
		}
		else if ($this->checkModuleAvailability($key))
		{
			return $this->register($key);
		}
		$return = $this->$key;

		return is_null($return) ? $default : $return; // can (and will be by default!) still be null
	}

	/**
	 *  Create a sub module of the current one
	 *  @name    instance
	 *  @type    method
	 *  @access  public
	 *  @param   string   modulename
	 *  @param   mixed    param N
	 *  @return  object
	 */
	public function instance($module)
	{
		switch ($module)
		{
			case 'GET':
			case 'POST':
			case 'PUT':
			case 'DELETE':
				if (!isset($this->_property[$module]))
					$this->_property[$module] = parent::instance('Type', $module);

				return $this->_property[$module];
				break;
		}
		$arg = func_get_args();
		return call_user_func_array(Array('parent', 'instance'), $arg);
	}

	/**
	 *  retrieve variables and assign them to the Request module
	 *  @name    _collect
	 *  @type    method
	 *  @access  protected
	 *  @return  void
	 */
	protected function _collect():void
	{
		$method = $_SERVER['REQUEST_METHOD'];

		switch ($method)
		{
			case 'POST':
			case 'PUT':
			case 'DELETE':
				$this->$method = parent::instance('Type', $method);
				//  no break, all of these requests may also have GET variables

			case 'GET':
				$this->GET = parent::instance('Type', 'GET');
				break;

			default:
				$this->call('/Log/message', 'Request-type ' . $method . ' not supported', 3);
				break;
		}

		$GLOBALS['_REQUEST'] = $this;
	}


	/*  ArrayAccess implementation */
	public function offsetGet($offset)
	{
		return $this->$offset;
	}

	public function offsetSet($offset, $value)
	{
		return $this->$offset = $value;
	}

	public function offsetExists($offset)
	{
		return isset($this->$offset);
	}

	public function offsetUnset($offset)
	{
		unset($this->$offset);
	}
}