<?hh  //  strict


class CoreStub<Konsolidate> extends Konsolidate {
	protected string $_name;

	/**
	 *  __construct, CoreStub constructor
	 *  @name    __construct
	 *  @type    constructor
	 *  @access  public
	 *  @param   object parent object
	 *  @param   string type [optional, default null - the request method used]
	 *  @return  object
	 */
	public function __construct(Konsolidate $parent, string $name, array $path) {
		parent::__construct($parent);

		$virtualPath = Array();
		foreach ($path as $tier=>$modulePath) {
			if ($modulePath && realpath($modulePath . '/' . strToLower($name))) {
				$virtualPath[$tier . ucFirst(strToLower($name))] = $modulePath . '/' . strToLower($name);
			}
		}

		$this->_name = $name;
		$this->_path = $virtualPath;
	}

	/**
	 *  Create a string representing the Konsolidate instance
	 *  @name    __toString
	 *  @type    method
	 *  @access  public
	 */
	public function __toString():string {
		$result  = '<div style="font-family:\'Lucida Grande\', Verdana, Arial, sans-serif;font-size:11px;color">';
		$result .= '<h3 style="margin:0;padding:0;">' . $this->_name . ' (' . get_class($this) . ')' . '</h3>';
		if (count($this->_property)) {
			$result .= '<div style="color:#400;">';
			$result .= '<em>Custom properties</em>';
			$result .= '<ul>';
			foreach ($this->_property as $key=>$value) {
				if (is_object($value)) {
					$result .= ' <li>' . $key . ' = (object ' . get_class($value) . ')</li>';
				}
				else if (is_array($value)) {
					$result .= ' <li>' . $key . ' = (array)</li>';
				}
				else if (is_bool($value)) {
					$result .= ' <li>' . $key . ' = (bool) ' . ($value ? 'true' : 'false') . '</li>';
				}
				else {
					$result .= ' <li>' . $key . ' = (' . gettype($value) . ') ' . $value . '</li>';
				}
			}
			$result .= '</ul>';
			$result .= '</div>';
		}

		if (count($this->_module)) {
			$result .= '<strong>Modules</strong>';
			$result .= '<ul>';
			foreach ($this->_module as $key=>$value)
				$result .= ' <li style="list-style-type:square;">' . $key . '<br />' . $value . '</li>';
			$result .= '</ul>';
		}
		$result .= '</div>';

		return $result;
	}
}
