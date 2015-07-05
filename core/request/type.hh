<?hh  //  strict


/**
 *  Unified access to request type buffers such as $_GET and $_POST
 *  @name    CoreRequestType
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreRequestType extends Konsolidate implements ArrayAccess {
	protected $_protect;
	protected $_verify;
	protected $_type;


	public function __construct(Konsolidate $parent, ?string $type=null) {
		parent::__construct($parent);

		$this->_type    = strToLower($type ?: $_SERVER['REQUEST_METHOD']);
		$this->_protect = $this->get('/Config/Request/protect_' . $this->_type, $this->get('/Config/Request/protect', true));
		$this->_verify  = $this->get('/Config/Request/verify_' . $this->_type, $this->get('/Config/Request/verify', true));

		$this->_collect();
	}

	/**
	 *  Enable overwrite protection for all class properties
	 *  @name    __set
	 *  @type    magic method
	 *  @access  public
	 *  @param   string key
	 *  @param   mixed  value
	 *  @return  void
	 */
	public function __set(string $name, mixed $value):void {
		if ($this->_protect) {
			return $this->call('/Log/message', __METHOD__ . ' not allowed to modify ' . strToUpper($this->_type) . ' request variables', 2);
		}

		return parent::__set($name, $value);
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
	protected function _populate(array $collection, string $buffer=null):void {
		foreach ($collection as $key=>$value) {
			$this->_property[$key] = $this->_verify ? $this->call('/Input/Verify/bufferValue', $buffer, $key, $value) : $value;
		}
	}

	/**
	 *  Obtain the request variables for the desired type
	 *  @name    _collect
	 *  @type    method
	 *  @access  protected
	 *  @return  void
	 */
	protected function _collect():void {
		//  determine the collection and try to populate it's properties
		switch ($this->_type) {
			//  use PHP's built-in _GET and/or _POST superglobals, override after copying
			case 'get':
			case 'post':
				$super = '_' . strToUpper($this->_type);
				if (isset($GLOBALS[$super]) && is_array($GLOBALS[$super])) {
					$buffer = $this->_type === 'get' ? $this->call('/Tool/serverVal', 'QUERY_STRING') : trim(file_get_contents('php://input'));
					$this->_populate($GLOBALS[$super], $buffer);
				}

				$GLOBALS[$super] = $this;
				break;

			//  provide PUT and DELETE support
			case 'put':
			case 'delete':
				$super = '_' . strToUpper($this->_type);
				if (isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] == strToUpper($this->_type)) {
					$raw = trim(file_get_contents("php://input"));
					if (!empty($raw)) {
						parse_str($raw, $temp);
						$this->_populate($temp, $raw);
					}
				}

				$GLOBALS[$super] = $this;
				break;

			default:
				$this->call('/Log/message', 'Unsupported request type: ' . $this->_type, 1);
				break;
		}
	}

	/*  ArrayAccess implementation */
	public function offsetGet(mixed $offset):mixed {
		return $this->{$offset};
	}

	public function offsetSet(mixed $offset, mixed $value):mixed {
		return $this->{$offset} = $value;
	}

	public function offsetExists(mixed $offset):bool {
		return isset($this->{$offset});
	}

	public function offsetUnset(mixed $offset):void {
		unset($this->{$offset});
	}
}
