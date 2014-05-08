<?hh  //  strict


/**
 *  Read and parse xml files and store it's sections/variables for re-use in the Config Module
 *  @name    CoreConfigXML
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreConfigXML<Konsolidate> extends Konsolidate
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
		$config = simplexml_load_file($file);

		if (is_object($config))
			return $this->_traverseXML($config, '/Config');

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
	protected function _traverseXML(SimpleXMLElement $node, string $path=null):bool
	{
		$result = true;

		if ($node->children())
			foreach ($node as $child)
				$result = $this->_traverseXML($child, $path . '/' . $node->getName()) && $result;
		else
			$result = $this->set($path . '/' . $node->getName(), (string) $node);

		return $result;
	}
}
