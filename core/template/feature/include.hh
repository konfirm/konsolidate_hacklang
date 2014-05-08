<?hh  //  strict


/**
 *  Include Template Feature, handles the <k:include /> feature which includes a subtemplate and places it at the position of the <k:include /> node
 *  @name    CoreTemplateFeatureInclude
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.net>
 */
class CoreTemplateFeatureInclude<CoreTemplateFeature> extends CoreTemplateFeature
{
	/**
	 *  Do all preparations needed for the feature to do its deed
	 *  @name   prepare
	 *  @type   method
	 *  @access public
	 *  @return bool success
	 */
	public function prepare():bool
	{
		$template = $this->instance('/Template', $this->file, $this->_template, false);
		$dom = $template->getDOM();

		foreach ($dom->documentElement->childNodes as $child)
		{
			$this->_node->parentNode->insertBefore(
				$this->_getDOMDocument()->importNode($child, true),
				$this->_node
			);
		}

		$this->_clean();
		$this->_template->prepare();

		return true;
	}
}
