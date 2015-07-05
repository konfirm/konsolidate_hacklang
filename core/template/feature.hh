<?hh  //  strict


/**
 *  Basic template feature class, associated with a DOMNode in the (XML) template
 *  @name    CoreTemplateFeature
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreTemplateFeature<Konsolidate> extends Konsolidate {
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
	public function __construct(Konsolidate $parent, DOMNode $node=null, CoreTemplate $template=null) {
		parent::__construct($parent);

		if (!empty($node)) {
			$this->_node = $node;
			foreach ($this->getAttributes() as $key=>$value) {
				$this->{$key} = $value;
			}
		}

		if (!empty($template)) {
			$this->_template = $template;
		}
	}

	/**
	 *  Do all preparations needed for the feature to do its deed
	 *  @name   prepare
	 *  @type   method
	 *  @access public
	 *  @return bool success
	 */
	public function prepare():bool {
		return true;
	}

	/**
	 *  Render the feature
	 *  @name   render
	 *  @type   method
	 *  @access public
	 *  @return bool success
	 */
	public function render():bool {
		if (get_class($this) === __CLASS__) {
			$this->_renderDummy();
		}

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
	public function value():string {
		return $this->_node->nodeValue;
	}

	/**
	 *  Return the DOMNode element which represents the feature
	 *  @name   node
	 *  @type   method
	 *  @access public
	 *  @return DOMNode node
	 */
	public function node():DOMNode {
		return $this->_node;
	}

	/**
	 *  Return a DOMElement or DOMText element to be used as offset
	 *  @name   offsetNode
	 *  @type   method
	 *  @access public
	 *  @return DOMNode
	 */
	public function offsetNode():DOMNode {
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
	public function attribute(string $name, string $value=null):?string {
		if (!is_null($value)) {
			$this->_node->setAttribute($name, $value);
		}

		return $this->_node->hasAttribute($name) ? $this->_node->getAttribute($name) : null;
	}

	/**
	 *  Obtain a key/value array of all the attributes
	 *  @name   getAttributes
	 *  @type   method
	 *  @access public
	 *  @return Map attributes
	 */
	public function getAttributes():Map {
		$result = Map {};
		foreach ($this->_node->attributes as $attribute) {
			$result[$attribute->nodeName] = $attribute->nodeValue;
		}

		return $result;
	}

	/**
	 *  Magic getter, adding a failsafe to look at the node attributes if no local property was found
	 *  @name   __get
	 *  @type   method
	 *  @access public (magic)
	 *  @param  string property
	 *  @return mixed value
	 */
	public function __get(string $property):mixed {
		$result = parent::__get($property);

		if (!$result) {
			$result = $this->attribute($property);
		}

		return $result;
	}

	/**
	 *  Obtain the DOMDocument in which the feature DOMNode resides
	 *  @name   _getDOMDocument
	 *  @type   method
	 *  @access protected
	 *  @return DOMDocument
	 */
	protected function _getDOMDocument():?DOMDocument {
		if ($this->_node instanceof DOMNode && $this->_node->ownerDocument) {
		 	return $this->_node->ownerDocument;
		 }
	}

	/**
	 *  Remove all evidence the feature was there
	 *  @name   _clean
	 *  @type   method
	 *  @access protected
	 *  @return void
	 */
	protected function _clean():void {
		if ($this->_node->parentNode) {
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
	protected function _renderDummy():void {
//		$source = '<div style="{style}"><h2>Unknown template feature: {name}</h2><div><h4>Origin</h4><p>The template containing the feature is: <pre>{origin}</pre></p><p>The feature code: <code>{source}</code></p></div><div><h4>Paths</h4><p>The feature was expected to be in one of the following paths:</p><ul><k:block name="path"><li>class <strong>{className}</strong> in {pathName}</li></k:block></ul></div><div><h4>Where to go from here</h4><p>Assuming the feature XML-syntax is correct, you now need to create its implementation class, this should be (one of):</p><ul><k:block name="suggest"><li><code>class {className} extends {extend}</code> in {pathName}</li></k:block></ul></div></div>';
		$source = '<div style="{style}"><h2>Unknown template feature: {name}</h2><div><h4>Origin</h4><p>The template containing the feature is: <pre>{origin}</pre></p><p>The feature code: <code>{source}</code></p></div></div>';

		$dom      = $this->_getDOMDocument();
		$name     = $this->_node->localName;
		$template = $this->instance('/Template', $source);
		$suggest  = true;

		$template->style  = 'font-family: sans-serif; font-size: 1em; line-height: 1.3em; border: 1px solid #4e342e; background-color: #ffab00; color: #4e342e; margin: 10px; padding: 10px;';
		$template->name   = $name;
		$template->origin = $this->_template->origin;
		$template->source = $dom->saveXML($this->_node);
/*
		foreach ($this->getFilePath() as $tier=>$path) {
			$className    = $tier . ucFirst($name);
			$pathName     = $path . '/' . $name . static::FILE_EXTENSION;
			$pathBlock    = $template->block('path');

			//  prevent suggestions to add any features to the core
			if ($suggest && preg_match('/^(?:core)/i', $tier)) {
				$suggest = false;
			}

			if ($pathBlock) {
				$pathBlock->className = $className;
				$pathBlock->pathName  = $pathName;
			}

			if ($suggest) {
				$suggestBlock = $template->block('suggest');
				if ($suggestBlock) {
					$suggestBlock->className = $className;
					$suggestBlock->pathName  = $pathName;
					$suggestBlock->extend    = __CLASS__;
				}
			}
		}
*/
		//  render the template into a DOMDocument
		if ($this->_node->parentNode) {
			$document = $template->render(true, true);
			$this->_node->parentNode->insertBefore($dom->importNode($document->documentElement, true), $this->_node);
		}
	}
}
