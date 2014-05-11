<?hh  //  strict


/**
 *  Read and parse json files and store it's sections/variables for re-use in the Config Module
 *  @name    CoreConfigJSON
 *  @type    class
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreConfigJSON<Konsolidate> extends Konsolidate
{
	/**
	 *  Load and parse an xml file and store it's sections/variables in the Konsolidate tree (the XML root node being the offset module)
	 *  @name    load
	 *  @type    method
	 *  @access  public
	 *  @param   string  xml file
	 *  @return  bool
	 */
	public function load(string $file):bool
	{
		$config = json_decode(file_get_contents($file));

		if (is_object($config))
			return $this->_traverseJSON($config, '/Config');

		return false;
	}

	/**
	 *  Traverse the XML tree and set all values in it, using the node structure as path
	 *  @name    _traverseXML
	 *  @type    method
	 *  @access  protected
	 *  @param   object  node
	 *  @param   string  xml file (optional, default null)
	 *  @return  bool
	 */
	protected function _traverseJSON(mixed $node, string $path=null):bool
	{
		$result = true;

		foreach ($node as $key=>$value)
		{
			if (is_object($value))
				$result = $this->_traverseJSON($value, $path . '/' . $key) && $result;
			else
				$this->set($path . '/' . $key, $value);
		}

		return $result;
	}
}