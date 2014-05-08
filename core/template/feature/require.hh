<?hh  //  strict


/**
 *  Require Template Feature, handles the <k:require /> feature which collects all external requirements such as javascripts and stylesheets
 *  @name    CoreTemplateFeatureRequire
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.net>
 */
class CoreTemplateFeatureRequire<CoreTemplateFeature> extends CoreTemplateFeature
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
		if ($this->attribute('file') && !$this->attribute('type'))
		{
			$type = $this->_getMIMEType(pathinfo($this->attribute('file'), PATHINFO_EXTENSION));
			if (!empty($type))
				$this->attribute('type', $type);
		}

		return true;
	}

	/**
	 *  Determine the MIME type by the file's extension
	 *  @name   _getMIMEType
	 *  @type   method
	 *  @access protected
	 *  @param  string file extension
	 *  @return string MIME type (null if not found)
	 */
	protected function _getMIMEType(string $extension):string
	{
		$type = null;
		switch ($extension)
		{
			case 'js':
				$type = 'text/javascript';
				break;

			case 'css':
				$type = 'text/css';
				break;
		}

		return $type;
	}
}
