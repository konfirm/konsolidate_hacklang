<?hh  //  strict


/**
 *  Basic template feature class, associated with a DOMNode in the (XML) template
 *  @name    CoreTemplateFeature
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.net>
 */
class CoreTemplateFeature<Konsolidate> extends Konsolidate
{
	protected DOMNode $_node;
	protected DOMNode $_placeholder;
	protected CoreTemplate $_template;


	/**
	 *  The constructor
	 *  @name   __construct
	 *  @type   method
	 *  @access public
	 *  @param  Konsolidate  parent
	 *  @param  DOMNode      node
	 *  @param  CoreTemplate template
	 */
	public function __construct(Konsolidate $parent, DOMNode $node=null, CoreTemplate $template=null)
	{
		parent::__construct($parent);

		if (!empty($node))
		{
			$this->_node = $node;
			foreach ($this->getAttributes() as $key=>$value)
				$this->{$key} = $value;
		}
		if (!empty($template))
			$this->_template = $template;
	}

	/**
	 *  Do all preparations needed for the feature to do its deed
	 *  @name   prepare
	 *  @type   method
	 *  @access public
	 *  @return bool success
	 */
	public function prepare():bool
	{
		return true;
	}

	/**
	 *  Render the feature
	 *  @name   render
	 *  @type   method
	 *  @access public
	 *  @return bool success
	 */
	public function render():bool
	{
		if (get_class($this) === __CLASS__)
			$this->_renderDummy();
		$this->_clean();
		return true;
	}

	/**
	 *  Return the DOMNode value
	 *  @name   value
	 *  @type   method
	 *  @access public
	 *  @return string node value
	 */
	public function value():string
	{
		return $this->_node->nodeValue;
	}

	/**
	 *  Return the DOMNode element which represents the feature
	 *  @name   node
	 *  @type   method
	 *  @access public
	 *  @return DOMNode node
	 */
	public function node():DOMNode
	{
		return $this->_node;
	}

	/**
	 *  Return a DOMElement or DOMText element to be used as offset
	 *  @name   offsetNode
	 *  @type   method
	 *  @access public
	 *  @return DOMNode
	 */
	public function offsetNode():DOMNode
	{
		return $this->_placeholder ? $this->_placeholder : $this->_node;
	}

	/**
	 *  Get (and/or set) an attribute value
	 *  @name   attribute
	 *  @type   method
	 *  @access public
	 *  @param  string attribute name
	 *  @param  string value (default null, if not null the value is set for the attribute)
	 *  @return string attribute value
	 */
	public function attribute(string $name, string $value=null):?string
	{
		if (!is_null($value))
			$this->_node->setAttribute($name, $value);
		return $this->_node->hasAttribute($name) ? $this->_node->getAttribute($name) : null;
	}

	/**
	 *  Obtain a key/value array of all the attributes
	 *  @name   getAttributes
	 *  @type   method
	 *  @access public
	 *  @return Map attributes
	 */
	public function getAttributes():Map
	{
		$return = Map {};
		foreach ($this->_node->attributes as $attribute)
			$return[$attribute->nodeName] = $attribute->nodeValue;

		return $return;
	}

	/**
	 *  Magic getter, adding a failsafe to look at the node attributes if no local property was found
	 *  @name   __get
	 *  @type   method
	 *  @access public (magic)
	 *  @param  string property
	 *  @return mixed value
	 */
	public function __get(string $property):mixed
	{
		$return = parent::__get($property);
		if (!$return)
			$return = $this->attribute($property);

		return $return;
	}

	/**
	 *  Obtain the DOMDocument in which the feature DOMNode resides
	 *  @name   _getDOMDocument
	 *  @type   method
	 *  @access protected
	 *  @return DOMDocument
	 */
	protected function _getDOMDocument():?DOMDocument
	{
		if ($this->_node instanceof DOMNode && $this->_node->ownerDocument)
		 	return $this->_node->ownerDocument;
	}

	/**
	 *  Remove all evidence the feature was there
	 *  @name   _clean
	 *  @type   method
	 *  @access protected
	 *  @return void
	 */
	protected function _clean():void
	{
		if ($this->_node->parentNode)
		{
			$dom = $this->_getDOMDocument();
			$this->_placeholder = $this->_node->parentNode->insertBefore(
				$dom->createTextNode(''),
				$this->_node
			);
			$this->_node->parentNode->removeChild($this->_node);
		}
	}

	/**
	 *  Render information about the template feature that was found in the template but does not have a class associated
	 *  @name    _renderDummy
	 *  @type    method
	 *  @access  protected
	 *  @return  void
	 */
	protected function _renderDummy():void
	{
		$dom  = $this->_getDOMDocument();
		$name = $this->_node->localName;
		$info = $this->_node->parentNode->insertBefore(
			$dom->createElement('div'),
			$this->_node
		);
		$info->setAttribute('style', 'border: 1px solid #900; background-color: #fd9; color: #900; margin: 20px; border-radius: 10px;');
		$info->appendChild($dom->createElement('h2', 'Unknown template feature: ' . $name));

		//  describe from where the feature request originated
		$origin = $info->appendChild($dom->createElement('div'));
		$origin->appendChild($dom->createElement('h4', 'Origin'));
		$origin->appendChild($dom->createElement('p', 'The template where the feature was called from is:'));
		$origin->appendChild($dom->createElement('code', $this->_template->origin));
		$origin->appendChild($dom->createElement('p', 'The exact feature syntax is:'));
		$origin->appendChild($dom->createElement('code', $dom->saveXML($this->_node)));

		//  describe where we've looked for the corresponding file
		$search = $info->appendChild($dom->createElement('div'));
		$search->appendChild($dom->createElement('h4', 'Paths'));
		$search->appendChild($dom->createElement('p', 'The feature was not found in the project libraries, expected one of:'));
		$list = $search->appendChild($dom->createElement('ul'));

		foreach ($this->getFilePath() as $tier=>$path)
		{
			$item = $list->appendChild($dom->createElement('li'));
			$item->appendChild($dom->createTextNode('class '));
			$item->appendChild($dom->createElement('strong', $tier . ucFirst($name)));
			$item->appendChild($dom->createTextNode(' in ' . $path . '/' . $name . '.class.php'));
		}

		$create = $info->appendChild($dom->createElement('div'));
		$create->appendChild($dom->createElement('h4', 'Where to go from here'));
		$create->appendChild($dom->createElement('p', 'Assuming the syntax of the feature is correct, you now need to create a feature implementation class, this should be (one of):'));
		$list = $create->appendChild($dom->createElement('ul'));
		foreach ($this->getFilePath() as $tier=>$path)
		{
			if (strpos($tier, 'Scaffold') === 0)
				break;
			$item = $list->appendChild($dom->createElement('li'));
			$item->appendChild($dom->createElement('code', 'class ' . $tier . ucFirst($name) . ' extends ' . __CLASS__));
			$item->appendChild($dom->createTextNode(' in ' . $path . '/' . $name . '.class.php'));
		}
	}
}
