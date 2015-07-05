<?hh  //  strict


/**
 *  Provide easy access to uploaded files
 *  @name    CoreRequestFile
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreRequestFile<Konsolidate> extends Konsolidate {
	/**
	 *  Move the uploaded file to target destination
	 *  @name    move
	 *  @type    method
	 *  @access  public
	 *  @param   string  destination
	 *  @param   bool    safe name [optional, default true]
	 *  @return  bool
	 */
	public function move(string $destination, bool $useSanitizedName=true):bool {
		if (is_uploaded_file($this->tmp_name)) {
			if (is_dir(realpath($destination)))  //  only directory provided, appending filename to it {
				$destination = realpath($destination) . '/' . ($useSanitizedName ? $this->sanitizedname : $this->name);
			}
			else if (!strstr(basename($destination), '.'))  //  assuming a dot in every filename... possible weird side effects? {
				mkdir($destination, 0777, true);
				$destination = realpath($destination) . '/' . ($useSanitizedName ? $this->sanitizedname : $this->name);
			}

			if (move_uploaded_file($this->tmp_name, $destination)) {
				unset($this->_property['tmp_name']);
				$this->location = $destination;
				return true;
			}
		}
		return false;
	}

	/**
	 *  Implicit set of properties
	 *  @name    __set
	 *  @type    method
	 *  @access  public
	 *  @param   string  key
	 *  @param   mixed   value
	 *  @return  void
	 *  @note    some additional properties are automaticalaly added when certain properties are set.
	 *           - 'error' also sets 'message', a string containing a more helpful error message.
	 *           - 'name' also set 'sanitizedname', a cleaned up (suggested) name for the file.
	 *           - 'tmp_name' also sets 'md5', the MD5 checksum of the file.
	 *           - 'size' also sets 'filesize', a human readable representation of the file size.
	 */
	public function __set(string $key, mixed $value):void {
		if (!empty($value) || $key == 'error') {
			parent::__set($key, $value);
			switch ($key) {
				case 'error':
					$this->_property['message'] = $this->_getErrorMessage($value);
					$this->_property['success'] = $value == UPLOAD_ERR_OK;
					break;

				case 'name':
					$this->_property['sanitizedname'] = preg_replace('/[^a-zA-Z0-9\._-]+/', '_', $value);
					break;

				case 'tmp_name':
					$this->_property['md5'] = md5_file($value);
					break;

				case 'size':
					$this->_property['filesize'] = $this->_bytesToLargestUnit($value);
					break;
			}
		}
	}


	/**
	 *  Convert byte size into something more readable for humans
	 *  @name    _bytesToLargestUnit
	 *  @type    method
	 *  @access  protected
	 *  @param   int     bytes
	 *  @return  string  readable unit
	 */
	protected function _bytesToLargestUnit(int $value):string {
		$result = $value . ' bytes';
		foreach (Array('KB', 'MB', 'GB', 'TB', 'PB') as $unit) {
			if ($value >= 1024) {
				$value /= 1024;
				$result  = (round($value * 10) / 10) . $unit;
			}
		}

		return $result;
	}

	/**
	 *  Resolve the UPLOAD_ERR_XX constant into its text representation (clarifying meaning)
	 *  @name    _getErrorMessage
	 *  @type    method
	 *  @access  protected
	 *  @param   int     error number
	 *  @return  string  error message
	 */
	protected function _getErrorMessage(int $error):string {
		$result = 'Unknown error';

		switch ($error) {
			case UPLOAD_ERR_OK:
				$result = 'No error';
				break;

			case UPLOAD_ERR_INI_SIZE:
				$result = 'The file exceeds PHP maximum file size';
				break;

			case UPLOAD_ERR_FORM_SIZE:
				$result = 'The file exceeds Form maximum file size';
				break;

			case UPLOAD_ERR_PARTIAL:
				$result = 'The file was only partially uploaded';
				break;

			case UPLOAD_ERR_NO_FILE:
				$result = 'No file was uploaded';
				break;

			case UPLOAD_ERR_NO_TMP_DIR:
				$result = 'Missing temporary upload location';
				break;

			case UPLOAD_ERR_CANT_WRITE:
				$result = 'Failed to write file to disk';
				break;

			case UPLOAD_ERR_EXTENSION:
				$result = 'File upload stopped by extension';
				break;
		};

		return $result;
	}
}
