<?hh  //  strict


/**
 *  Block Template Feature, handles the <k:block /> feature which allow for repeating blocks of (x)html
 *  @name    CoreTemplateFeatureBlock
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreTemplateFeatureBlock<CoreTemplateFeature> extends CoreTemplateFeature
{
	protected DOMNode $_marker;
	protected string $_data;
	protected Vector<CoreTemplate> $_stack;


	/**
	 *  Do all preparations needed for the feature to do its deed
	 *  @name   prepare
	 *  @type   method
	 *  @access public
	 *  @return bool success
	 */
	public function prepare():bool
	{
		$this->_marker = $this->_node->parentNode->insertBefore(
			$this->_getDOMDocument()->createComment('block \'' . $this->name . '\''),
			$this->_node
		);

		$data = '';
		$dom  = $this->_getDOMDocument();
		foreach ($this->_node->childNodes as $child)
			$data .= trim($dom->saveXML($child));
		$this->_data = $data;

		$this->_node->parentNode->removeChild($this->_node);

		return true;
	}

	/**
	 *  Duplicate the block features content
	 *  @name   duplicate
	 *  @type   method
	 *  @access public
	 *  @return Template object
	 */
	public function duplicate():CoreTemplate
	{
		$instance = $this->instance('/Template');
		$instance->load($this->_data, $this->_template);

		return $this->_addToStack($instance);
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
		$this->_renderStack();
		$this->_marker->parentNode->removeChild($this->_marker);

		return true;
	}

	/**
	 *  Add the given template to the internal stack of duplicated blocks
	 *  @name   _addToStack
	 *  @type   method
	 *  @access protected
	 *  @param  Template object
	 *  @return Template object
	 *  @note   The given template object will also be prepared with some predefined variables
	 */
	protected function _addToStack(CoreTemplate $template):CoreTemplate
	{
		if (!$this->_stack)
			$this->_stack = Vector<CoreTemplate> {};

		$this->_stack->add($this->_getPopulatedTemplate($template, count($this->_stack)));

		return $template;
	}

	/**
	 *  Render the internal stack of duplicated blocks
	 *  @name   _renderStack
	 *  @type   method
	 *  @access protected
	 *  @return void
	 */
	protected function _renderStack():void
	{
		if ($this->_stack)
			foreach ($this->_stack as $template)
			{
				$dom = $template->render(true, true);
				foreach ($dom->childNodes as $child)
					$this->_marker->parentNode->insertBefore(
						$this->_marker->ownerDocument->importNode($child, true),
						$this->_marker
					);
			}
	}

	/**
	 *  Prefill the given template with default variables
	 *  @name   _getPopulatedTemplate
	 *  @type   method
	 *  @access protected
	 *  @return Template object
	 */
	protected function _getPopulatedTemplate(CoreTemplate $template, int $index):CoreTemplate
	{
		$template->_position = $index;
		$template->_parity   = $index % 2 == 0 ? 'even' : 'odd';
		$template->_name     = $this->name;

		return $template;
	}
}
