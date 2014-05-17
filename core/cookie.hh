<?hh  //  strict


/**
 *  Unified access to cookies, validating $_COOKIE variables
 *  @name    CoreCookie
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreCookie<Konsolidate> extends Konsolidate implements ArrayAccess
{
	protected bool $_verify;


	public function __construct(Konsolidate $parent)
	{
		parent::__construct($parent);

		$this->_verify = $this->get('/Config/Cookie/verify', $this->get('/Config/Request/verify', true));
		$this->_collect();
	}

	/**
	 *  set a property in a module using a path
	 *  @name    set
	 *  @type    method
	 *  @access  public
	 *  @param   string   path to the property to set
	 *  @param   mixed    value
	 *  @param   integer  expire   (optional, default end of session)
	 *  @param   string   path     (optional, default current path)
	 *  @param   string   domain   (optional, default current domain)
	 *  @param   bool     secure   (optional, default false)
	 *  @param   bool     httpOnly (options, default false)
	 *  @return  void
	 */
	public function set():void
	{
		$argument  = func_get_args();
		$property  = array_shift($argument);
		$separator = strrpos($property, static::MODULE_SEPARATOR);

		if ($separator !== false && ($instance = $this->getModule(substr($property, 0, $separator))) !== false)
		{
			array_unshift($argument, substr($property, $separator + 1));

			return call_user_func_array(Array($instance, 'set'), $argument);
		}

		$value    = array_shift($argument);
		$expires  = count($argument) ? array_shift($argument) : null;
		$path     = count($argument) ? array_shift($argument) : null;
		$domain   = count($argument) ? array_shift($argument) : null;
		$secure   = count($argument) ? array_shift($argument) : false;
		$httpOnly = count($argument) ? array_shift($argument) : false;

		$this->_removeCounterfit($property);
		if (setCookie($property, $value, $expires, $path, $domain, $secure, $httpOnly))
			$this->_property[$property] = $value;

		return $this->$property === $value;
	}

	/**
	 *  Enable shorthand cookie setting
	 *  @name    __set
	 *  @type    magic method
	 *  @access  public
	 *  @param   string key
	 *  @param   mixed  value
	 *  @return  void
	 */
	public function __set(string $property, mixed $value):void
	{
		$this->_removeCounterfit($property);
		setCookie($property, $value);

		return parent::__set($property, $value);
	}

	/**
	 *  Enable shorthand cookie removal
	 *  @name    __unset
	 *  @type    magic method
	 *  @access  public
	 *  @param   string key
	 *  @return  void
	 */
	public function __unset(string $property):void
	{
		$this->_removeCounterfit($property);

		if (isset($this->_property[$property]))
		{
			setCookie($property, '');
			unset($this->_property[$property]);
		}
	}

	/**
	 *  Obtain the cookie variables
	 *  @name    _collect
	 *  @type    method
	 *  @access  protected
	 *  @return  void
	 */
	protected function _collect():void
	{
		$super = '_COOKIE';
		if (isset($GLOBALS[$super]) && is_array($GLOBALS[$super]))
			$this->_populate($GLOBALS[$super], $this->call('/Tool/serverVal', 'HTTP_COOKIE'));

		$GLOBALS[$super] = $this;
	}

	/**
	 *  Populate the class properties from the collection (and verify the values if _verify is on)
	 *  @name    _populate
	 *  @type    method
	 *  @access  protected
	 *  @param   array   collection
	 *  @param   string  buffer (to verify against)
	 *  @return  void
	 */
	protected function _populate(array $collection, ?string $buffer=null):void
	{
		foreach ($collection as $key=>$value)
			$this->_property[$key] = $this->_verify ? $this->call('/Input/Verify/bufferValue', $buffer, $key, $value, ';', false) : $value;
	}

	/**
	 *  Look for any cookie which would end up internally using the exact same notation but uses a different syntax
	 *  in the cookie header (e.g. 'session_id' and 'session[id' would both end up as 'session_id', hence we remove
	 *  the 'session[id'
	 *  @name    _removeCounterfit
	 *  @type    method
	 *  @access  protected
	 *  @param   string  property
	 *  @return  void
	 */
	protected function _removeCounterfit(string $property):void
	{
		if (strpos($property, '_') !== false && preg_match_all('/(' . str_replace('_', '[\[_]', $property) . ')=[^;]*/', $this->call('/Tool/serverVal', 'HTTP_COOKIE'), $match))
			foreach ($match[1] as $pattern)
				if ($pattern !== $property)
					setCookie($pattern, '');
	}



	/*  ArrayAccess implementation */
	public function offsetGet(mixed $offset):mixed
	{
		return $this->{$offset};
	}

	public function offsetSet(mixed $offset, mixed $value):mixed
	{
		return $this->{$offset} = $value;
	}

	public function offsetExists(mixed $offset):mixed
	{
		return isset($this->{$offset});
	}

	public function offsetUnset(mixed $offset):mixed
	{
		unset($this->{$offset});
	}
}