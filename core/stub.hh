<?hh  //  strict


class CoreStub<Konsolidate> extends Konsolidate
{
	protected string $_name;

	public function __construct($parent, $name, $path)
	{
		parent::__construct($parent);

		$path = array_walk($path, function(&$p, $k, $class):void{
			$p = realpath($p . '/' . $class);
		}, $name);
//

		$this->_name = $name;
		$this->_path = $this->_filterPathList(is_array($path) ? $path : []);
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
		$return .= '<h3 style="margin:0;padding:0;">' . $this->_name . ' (' . get_class($this) . ')' . '</h3>';
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
