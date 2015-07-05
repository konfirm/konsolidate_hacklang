<?hh  //  strict


/**
 *  Style Template Feature, handles the <k:style /> feature which collects all stylesheet requirements and removes duplicates
 *  @name    CoreTemplateFeatureStyle
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreTemplateFeatureStyle<CoreTemplateFeature> extends CoreTemplateFeature {
	/**
	 *  Render the feature
	 *  @name   render
	 *  @type   method
	 *  @access public
	 *  @return bool success
	 */
	public function render():bool {
        $files    = Map<string, bool> {};
		$requires = $this->_template->getFeatures('require', Array('type'=>'text/css'), true);
		$dom      = $this->_getDOMDocument();

		foreach ($requires as $requirement) {
			//  requirements referencing an external file will be included only once unless the multiple="true" attribute is set
			if (isset($requirement->file)) {
				if ($files->contains($requirement->file) && $requirement->multiple !== 'true') {
					continue;
				}
				else {
					$files->add(Pair {$requirement->file, true});
				}
			}

			//  if the require feature has been fixated (either the template author added an attribute fixate="true" or
			//  the feature class was overruled and it was fixated in the extending class), use the require feature
			//  element as offset, otherwise the current feature element is used as offset (effectively collecting the
			//  elements in one place)
			$offset = $requirement->fixate == 'true' ? $requirement->offsetNode() : $this->_node;

			//  if the requirement has the file property, we need to reference it differently
			if (isset($requirement->file)) {
				//  create a link element right before the offset element
				$node = $offset->parentNode->insertBefore(
					$dom->createElement('link'),
					$offset
				);
				$node->setAttribute('rel', 'stylesheet');
				$node->setAttribute('type', 'text/css');
				$node->setAttribute('href', $requirement->file);
			}
			else {
				//  obtain the source and see if it has content
				$source = $requirement->value();
				if (!empty($source)) {
					//  create a style element right before the offset element, minify the source and append it to the new element
					$node = $offset->parentNode->insertBefore(
						$dom->createElement('style', $this->call('/Source/Style/minify', $source)),
						$offset
					);
					$node->setAttribute('type', 'text/css');
				}
			}
		}

		//  let the extended class do its rendering
		return parent::render();
	}
}
