<?hh  //  strict


class Konsolidate<T> implements Iterator
{
	const MODULE_SEPARATOR = '/';

	protected ?Konsolidate $_parent;
	protected bool $_debug;
	protected array<string, mixed> $_module;
	protected array<string, mixed> $_property;
	protected array<string, string> $_path;
	protected array<string, Konsolidate> $_lookupcache;
	static protected bool $_modulecheck;
	protected array<string> $_tracelog;


	public function __construct(mixed $path)
	{
		$this->_debug       = false;
		$this->_module      = Array();
		$this->_property    = Array();
		$this->_lookupcache = Array();
		$this->_tracelog    = Array();

		if (is_object($path) && $path instanceof Konsolidate)
		{
			$this->_parent = $path;
			$this->_path   = $this->getFilePath();
		}
		else if (is_array($path))
		{
			foreach ($path as $tier=>$directory)
				$path[$tier] = realpath($directory);

			$this->_path = $this->_filterPathList($path);
		}
	}

	/**
	 *  get a property value from a module using a path
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
		$arg     = func_get_args();
		$pty     = array_shift($arg);
		$default = count($arg) ? array_shift($arg) : null;

		$separator = strrpos($pty, static::MODULE_SEPARATOR);
		if ($separator !== false && ($module = $this->getModule(substr($pty, 0, $separator))) !== false)
			return $module->get(substr($pty, $separator + 1), $default);
		else if ($this->checkModuleAvailability($pty))
			return $this->register($pty);
		$return = $this->$pty;

		return is_null($return) ? $default : $return; // can (and will be by default!) still be null
	}

	/**
	 *  set a property in a module using a path
	 *  @name    set
	 *  @type    method
	 *  @access  public
	 *  @param   string   path to the property to set
	 *  @param   mixed    value
	 *  @return  void
	 */
	public function set():void
	{
		$arg  = func_get_args();
		$pty  = array_shift($arg);
		$separator = strrpos($pty, static::MODULE_SEPARATOR);
		if ($separator !== false && ($module = $this->getModule(substr($pty, 0, $separator))) !== false)
		{
			array_unshift($arg, substr($pty, $separator + 1));
			return call_user_func_array(Array($module, 'set'), $arg);
		}

		$value      = array_shift($arg);
		$this->$pty = $value;
		return $this->$pty === $value;
	}

	/**
	 *  Call a method from a module and return its return value
	 *  @name    call
	 *  @type    method
	 *  @access  public
	 *  @param   string   path to the method to call
	 *  @param   mixed    [optional] argument
	 *  @return  mixed
	 *  @note    One can supply as many arguments as needed
	 */
	public function call():mixed
	{
		$arg       = func_get_args();
		$call      = array_shift($arg);
		$separator = strrpos($call, static::MODULE_SEPARATOR);

		if ($separator !== false)
		{
			$module = $this->getModule(substr($call, 0, $separator));
			$method = substr($call, $separator + 1);
		}
		else
		{
			$module = $this;
			$method = $call;
		}

		if (!is_object($module))
		{
			$this->call('/Log/write', 'Module "' . get_class($module) . '" not found!');
			return false;
		}

		return call_user_func_array(Array($module, $method), $arg);
	}


	/**
	 *  Get a reference to a module based on it's path
	 *  @name    getModule
	 *  @type    method
	 *  @access  public
	 *  @param   string  module path
	 *  @return  mixed
	 */
	public function getModule(string $path):Konsolidate
	{
		$path = strToUpper($path);
		if (!array_key_exists($path, $this->_lookupcache))
		{
			$segments = explode(static::MODULE_SEPARATOR, $path);
			$module   = $this;
			while (is_object($module) && count($segments))
			{
				$part = array_shift($segments);
				switch( strToLower($part))
				{
					case '':       //  root
					case '_root':
						$traverse = $module->getRoot();
						break;

					case '..':      //  parent
					case '_parent': //
						$traverse = $module->getParent();
						break;

					case '.':       //  self
						$traverse = $this;
						break;

					default:        //  child
						$traverse = $module->register($part);
						break;
				}

				if (!is_object($traverse))
				{
					throw new Exception('Cannot find module ' . $path . ' (got lost at: ' . $part . ')');
//					$this->call('/Log/write', 'Module "' . $part .'"" not found in module "' . get_class($module) . '"!', 3);
					return $traverse;
				}

				$module = $traverse;
			}

			$this->_lookupcache[$path] = $module;
		}

		return $this->_lookupcache[$path];
	}

	/**
	 *  Register a (unique) sub module of the current one
	 *  @name    register
	 *  @type    method
	 *  @access  public
	 *  @param   string   modulename
	 *  @return  object
	 *  @note    register only create a single (unique) instance and always returns the same instance
	 *           use the instance method to create different instances of the same class
	 */
	public function register($module):?Konsolidate
	{
		$module = strToUpper($module);
		if (!array_key_exists($module, $this->_module))
		{
			$instance = $this->instance($module);

			if ($instance === false)
				return $instance;

			$this->_module[$module] = $instance;
		}

		return $this->_module[$module];
	}

	/**
	 *  Create a sub module of the current one
	 *  @name    instance
	 *  @type    method
	 *  @access  public
	 *  @param   string   modulename
	 *  @param   mixed    param N
	 *  @return  object
	 *  @note    instance creates an instance every time you call it, if you require a single instance which
	 *           is always returned, use the register method
	 */
	public function instance($module):?Konsolidate
	{
		$instance = null;
		//  In case we request an instance of a remote node, we verify it here and leave the instancing to the instance parent
		$separator = strrpos($module, static::MODULE_SEPARATOR);
		if ($separator !== false && ($instance = $this->getModule(substr($module, 0, $separator))) !== false)
		{
			$arg = func_get_args();
			if (count($arg))
			{
				$arg[0] = substr($arg[0], $separator + 1);
				return call_user_func_array(Array($instance, 'instance'), $arg);
			}
		}

		//  optimize the number of calls to import, as importing is rather expensive due to the file I/O involved
		static $imported = Array();
		if (!isset($imported[$module]))
		{
			$imported[$module] = microtime(true);
			$this->import($module . '.hh');
		}

		//  try to construct the module classes top down, this ensures the correct order of construction
		$constructed = false;
		foreach ($this->_path as $tier=>$path)
		{
			$class  = $tier . ucFirst(strToLower($module));

			if (class_exists($class))
			{
				$arg = func_get_args();
				array_shift($arg);  //  the first argument is always the module to instance, we discard it

				if (count($arg))
				{
					array_unshift($arg, $this); //  inject the 'parent reference', as Konsolidate dictates
					$instance = new ReflectionClass($class);
					$instance = $instance->newInstanceArgs($arg);
				}
				else
				{
					$instance = new $class($this);
				}
				break;
			}
		}

		if (!$instance && strtolower($module) !== 'stub')
		{
			//  unlike PHP, we cannot create a class dynamically in Hack, so we resort
			//  to a dedicated Stub class which resides in the Core.
			$this->getRoot()->import('stub.hh');
			$instance = new CoreStub($this, $module, $this->_path);
		}

		return $instance;
	 }


	public function checkModuleAvailability($module)
	{
		$module = strtolower($module);
		$class  = get_class($this);

		if (!isset(static::$_modulecheck[$class]))
			$this->_indexModuleAvailability();

		//  if we are dealing with a submodule pattern which is not in our cache by default, test for it
		if (strpos($module, static::MODULE_SEPARATOR) !== false)
			foreach ($this->_path as $tier=>$path)
			{
				$modulePath = $path . '/' . $module;
				if (realpath($modulePath) || realpath($modulePath . '.class.php'))
				{
					static::$_modulecheck[$class][$module] = true;
					break;
				}
			}

		return isset(static::$_modulecheck[$class][$module]) ? static::$_modulecheck[$class][$module] : false;
	}

	/**
	 *  Get the root node
	 *  @name    getRoot
	 *  @type    method
	 *  @access  public
	 *  @return  Konsolidate object
	 */
	public function getRoot():Konsolidate
	{
		return $this->_parent instanceof Konsolidate ? $this->_parent->getRoot() : $this;
	}

	/**
	 *  Get the parent node, if any
	 *  @name    getParent
	 *  @type    method
	 *  @access  public
	 *  @return  Konsolidate object (null if no parent is available (the root object))
	 */
	public function getParent():?Konsolidate
	{
		return $this->_parent;
	}

	public function import($file)
	{
		$separator = strrpos($file, static::MODULE_SEPARATOR);
		if ($separator !== false && ($module = $this->getModule(substr($file, 0, $separator))) !== false)
			return $module->import(substr($file, $separator + 1));

		//  include all imported files (if they exist) bottom up, this solves the implementation classes having to know core paths
		$included = array_flip(get_included_files());
		$pathList = array_reverse($this->_path, true);
		$imported = false;
		foreach ($pathList as $path)
		{
			$current = $path . '/' . strToLower($file);
			if (isset($included[$current]))
			{
				$imported = true;
			}
			else if (realpath($current))
			{
				include($current);
				$imported = true;
			}
		}
		return $imported;
	}

	public function exception(string $message, int $code=0):void
	{
		$this->import('exception.hh');
		$class = str_replace(array_keys($this->getRoot()->getFilePath(), '', get_class($this) . 'Exception'));
		$throw = class_exists($class) ? $class : null;

		if (empty($throw))
			foreach ($this->_path as $tier=>$path)
			{
				$exist = $tier . 'Exception';
				if (class_exists($exist))
				{
					$throw = $exist;
					break;
				}
			}

		if (empty($throw))
			$throw = 'Exception';

		$trace = debug_backtrace();
		$file  = !empty($trace[3]) ? $trace[3]['file'] : 'Unknown file';
		$line  = !empty($trace[3]) ? $trace[3]['line'] : 'Unknown line';

		throw new $throw($message, $code);
	}

	/**
	 *  Get the file path based on the location in the Konsolidate Tree
	 *  @name    getFilePath
	 *  @type    method
	 *  @access  public
	 *  @return  array<string, string> path
	 */
	public function getFilePath():array<string, string>
	{
		if (is_array($this->_path))
			return $this->_path;

		$parentPath = $this->_parent->getFilePath();
		$class      = strtolower(str_replace(array_keys($parentPath), '', get_class($this)));
		$pathList   = Array();

		foreach ($parentPath as $tier=>$path)
		{
			$classPath = $path . '/' . strToLower($class);
			if (realpath($classPath))
				$pathList[$tier . $class] = $classPath;
		}

		return $pathList ? $this->_filterPathList($pathList) : [];
	}


	public function getTopAuthoredClass():string
	{
		if (property_exists($this, '_dynamicStubClass'))
			return $this->call('../getTopAuthoredClass');
		return get_class($this);
	}

	/**
	 *  Filter an array of paths so only existing paths are maintains
	 *  @name    _filterPathList
	 *  @type    method
	 *  @access  protected
	 *  @return  array<string, string> path
	 */
	protected function _filterPathList(array<string, string> $path):array<string, string>
	{
		return array_filter($path, function($p){
			return realpath($p);
		});
	}

	/**
	 *  Look ahead at all available submodules and cache the availability
	 *  @name    _indexModuleAvailability
	 *  @type    method
	 *  @access  protected
	 *  @returns void
	 */
	protected function _indexModuleAvailability():void
	{
		if (!is_array(static::$_modulecheck))
			static::$_modulecheck = Array();

		$class = get_class($this);
		if (!isset(static::$_modulecheck[$class]))
		{
			$list = Array();
			if (is_array($this->_path))
				foreach ($this->_path as $tier=>$path)
					foreach (glob($path . '/*') as $item)
						$list[strtolower(basename($item, '.class.php'))] = true;
			static::$_modulecheck[$class] = $list;
		}
	}

	//  Iterator functionality
	public function key():T
	{
		return key($this->_property);
	}

	public function current():T
	{
		return current($this->_property);
	}

	public function next():T
	{
		return next($this->_property);
	}

	public function rewind():T
	{
		return reset($this->_property);
	}

	public function valid():bool
	{
		return !is_null($this->key());
	}
	//  End Iterator functionality



	// Magic methods.
	public function __set(string $pty, ?mixed $value):void
	{
		if (array_key_exists(strToUpper($pty), $this->_module))
			throw new Exception('Trying to overwrite existing module ' . $pty . ' in ' . get_class($this) . ' with ' . gettype($value) . ' ' . $value);
		else if ($this->checkModuleAvailability($pty))
			throw new Exception('Trying to set a property ' . gettype($value) . ' ' . $value . ' in ' . get_class($this) . ' where a module with the same name is available');
		$this->_property[$pty] = $value;
	}

	public function __get($pty):T
	{
		if ($pty == 'modules')
			return $this->_module;
		else if (array_key_exists($pty, $this->_property))
			return $this->_property[$pty];
		else if (array_key_exists(strToUpper($pty), $this->_module))
			return $this->_module[strToUpper($pty)];
		else if ($this->checkModuleAvailability($pty))
			return $this->get($pty);
		return null;
	}

	public function __call(string $method, ?array $arg):T
	{
		$self     = get_class($this);
		$authored = $this->getTopAuthoredClass();
		$message  = 'Call to unknown method "' . $self . '::' . $method . '" '. ( $authored !== $self ? ', nearest authored class is "' . $authored . '"' : '');
		$this->call('/Log/write', $message, 0);
		throw new Exception($message);
		return false;
	}

	/**
	 *  Allow Konsolidate and its module instances to be invoked directly
	 *  @name    __invoke
	 *  @type    magic method
	 *  @access  public
	 *  @param   mixed   arg N
	 *  @return  mixed
	 *  @note    You can now effectively leave out the '->call' part when calling on methods, 
	 *           e.g. $oK('/DB/query', 'SHOW TABLES') instead of $oK->call('/DB/query', 'SHOW TABLES');
	 *  @see     call
	 */
	public function __invoke():T
	{
		return call_user_func_array(Array($this, 'call'), func_get_args());
	}

	/**
	 *  Allow isset/empty tests on inaccessible properties
	 *  @name    __isset
	 *  @type    magic method
	 *  @access  public
	 *  @param   string property
	 *  @return  bool isset
	 */
	public function __isset(string $pty):bool
	{
		return isset($this->_property[$pty]);
	}

	/**
	 *  Allow unsetting of inaccessible properties
	 *  @name    __unset
	 *  @type    magic method
	 *  @access  public
	 *  @param   string property
	 */
	public function __unset(string $pty):void
	{
		unset($this->_property[$pty]);
	}

	/**
	 *  Create a string representing the Konsolidate instance
	 *  @name    __toString
	 *  @type    method
	 *  @access  public
	 */
	public function __toString()
	{
		$return  = '<div style="font-family:\'Lucida Grande\', Verdana, Arial, sans-serif;font-size:11px;color">';
		$return .= '<h3 style="margin:0;padding:0;">' . get_class($this) . '</h3>';
		if (count($this->_property))
		{
			$return .= '<div style="color:#400;">';
			$return .= '<em>Custom properties</em>';
			$return .= '<ul>';
			foreach ($this->_property as $key=>$value)
				if (is_object($value))
					$return .= ' <li>' . $key . ' = (object ' . get_class($value) . ')</li>';
				else if (is_array($value))
					$return .= ' <li>' . $key . ' = (array)</li>';
				else if (is_bool($value))
					$return .= ' <li>' . $key . ' = (bool) ' . ($value ? 'true' : 'false') . '</li>';
				else
					$return .= ' <li>' . $key . ' = (' . gettype($value) . ') ' . $value . '</li>';
			$return .= '</ul>';
			$return .= '</div>';
		}
		if (count($this->_module))
		{
			$return .= '<strong>Modules</strong>';
			$return .= '<ul>';
			foreach ($this->_module as $key=>$value)
				$return .= ' <li style="list-style-type:square;">' . $key . '<br />' . $value . '</li>';
			$return .= '</ul>';
		}
		$return .= '</div>';

		return $return;
	}

}
