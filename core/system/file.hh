<?hh  //  strict


/**
 *  File IO, either use an instance to put/get or read/write an entire file at once
 *  @name    CoreSystemFile
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreSystemFile<Konsolidate> extends Konsolidate
{
	const SCOPE_PROTECTION = DOCUMENT_ROOT;

	protected $_filepointer;

	/**
	 *  Read an entire file and return the contents
	 *  @name    read
	 *  @type    method
	 *  @access  public
	 *  @param   string filename
	 *  @return  string file contents (bool false on error)
	 */
	public function read(string $file):mixed
	{
		$this->_verifyScope($file);

		return file_exists($file) && is_readable($file) ? file_get_contents($file) : false;
	}

	/**
	 *  Write data to a file
	 *  @name    write
	 *  @type    method
	 *  @access  public
	 *  @param   string filename
	 *  @param   mixed  data [will be cast to string]
	 *  @return  bool success
	 */
	public function write(string $file, mixed $data):bool
	{
		$this->_verifyScope($file);

		return file_put_contents($file, (string) $data);
	}

	/**
	 *  Set the mode of a file
	 *  @name    mode
	 *  @type    method
	 *  @access  public
	 *  @param   string filename
	 *  @param   number mode
	 *  @return  bool success
	 *  @note    mode needs be an octal number, eg 0777
	 */
	public function mode(string $file, int $mode):bool
	{
		$this->_verifyScope($file);

		return chmod($file, $mode);
	}

	/**
	 *  unlink(delete)a file
	 *  @name    unlink
	 *  @type    method
	 *  @access  public
	 *  @param   string filename
	 *  @return  bool success
	 */
	public function unlink(string $file):bool
	{
		$this->_verifyScope($file);

		return unlink($file);
	}

	/**
	 *  delete a file
	 *  @name    delete
	 *  @type    method
	 *  @access  public
	 *  @param   string filename
	 *  @return  bool success
	 *  @see     unlink
	 *  @note    an alias method for unlink
	 */
	public function delete(string $file):bool
	{
		$this->_verifyScope($file);

		return $this->unlink($file);
	}

	/**
	 *  rename a file
	 *  @name    rename
	 *  @type    method
	 *  @access  public
	 *  @param   string filename
	 *  @param   string newfilename
	 *  @param   bool   force(optional, default false)
	 *  @return  bool success
	 */
	public function rename(string $file, string $newFile, bool $force=false):bool
	{
		$this->_verifyScope($file);

		return file_exists($file) && ($force || (!$force && !file_exists($newFile))) ? rename($file, $newFile) : false;
	}


	/**
	 *  Open a file for interaction
	 *  @name    open
	 *  @type    method
	 *  @access  public
	 *  @param   string filename
	 *  @param   string mode(optional, default "r"(read access))
	 *  @return  bool success
	 *  @note    Warning: Since you cannot know if your code is the only code currently accessing any file
	 *           you can best create a unique instance to use this method, obtained through:[KonsolidateObject]->instance("/System/File");
	 */
	public function open(string $file, $mode='r'):bool
	{
		$this->_verifyScope($file);

		$this->_filepointer = fopen($file, $mode);
		return $this->_filepointer !== false;
	}

	/**
	 *  Get data from an opened file
	 *  @name    get
	 *  @type    method
	 *  @access  public
	 *  @param   mixed  int length[optional, default 4096 bytes], or string property
	 *  @return  mixed  data
	 *  @note    If a string property is provided, the property value is returned, otherwise the next line of the opened file is returned.
	 *           Warning: Since you cannot know if your code is the only code currently accessing any file
	 *           you can best create a unique instance to use this method, obtained through:[KonsolidateObject]->instance("/System/File");
	 */
	public function get():mixed
	{
		//  in order to achieve compatiblity with Konsolidates set method, the params are read 'manually'
		$arg     = func_get_args();
		$length  = count($arg) ? array_shift($arg) : 4096;
		$default = count($arg) ? array_shift($arg) : null;

		if (is_integer($length))
			return is_resource($this->_filepointer) && !feof($this->_filepointer) ? fgets($this->_filepointer, $length) : false;

		return parent::get($length, $default);
	}

	/**
	 *  Put data into an opened file
	 *  @name    put
	 *  @type    method
	 *  @access  public
	 *  @param   string data
	 *  @return  bool success
	 *  @note    Warning: Since you cannot know if your code is the only code currently accessing any file
	 *           you can best create a unique instance to use this method, obtained through:[KonsolidateObject]->instance("/System/File");
	 */
	public function put(mixed $data):bool
	{
		$data = (string) $data;

		return is_resource($this->_filepointer) ? fputs($this->_filepointer, $data, strlen($data)) : false;
	}

	/**
	 *  Get data from an opened file
	 *  @name    next
	 *  @type    method
	 *  @access  public
	 *  @return  string data
	 *  @see     get
	 *  @note    Alias of get, relying on the default amount of bytes
	 *  @note    Warning: Since you cannot know if your code is the only code currently accessing any file
	 *           you can best create a unique instance to use this method, obtained through:[KonsolidateObject]->instance("/System/File");
	 */
	public function next():mixed
	{
		return $this->get();
	}

	/**
	 *  Close the opened file
	 *  @name    close
	 *  @type    method
	 *  @access  public
	 *  @return  bool success
	 *  @note    Warning: Since you cannot know if your code is the only code currently accessing any file
	 *           you can best create a unique instance to use this method, obtained through:[KonsolidateObject]->instance("/System/File");
	 */
	public function close():bool
	{
		return is_resource($this->_filepointer) ? fclose($this->_filepointer) : false;
	}

	/**
	 *  Get the filepointer of the opened file
	 *  @name    getFilePointer
	 *  @type    method
	 *  @access  public
	 *  @return  resource filepointer
	 *  @note    Warning: Since you cannot know if your code is the only code currently accessing any file
	 *           you can best create a unique instance to use this method, obtained through:[KonsolidateObject]->instance("/System/File");
	 */
	public function getFilePointer():resource
	{
		return $this->_filepointer;
	}



	/**
	 *  Magic __destruct, closes open filepointers
	 *  @name    __destruct
	 *  @type    method
	 *  @access  public
	 *  @return  void
	 */
	public function __destruct():void
	{
		if (!is_null($this->_filepointer) && is_resource($this->_filepointer))
			$this->close();
	}

	/**
	 *  Verify is the file at hand is within the scope defined in SCOPE_PROTECTION
	 *  @name    _verifyScope
	 *  @type    method
	 *  @access  protected
	 *  @return  void
	 */
	protected function _verifyScope(string $file):void
	{
		$protect = static::SCOPE_PROTECTION;

		if ($protect && strpos($file, '/') === 0 && substr($file, 0, strlen($protect)) !== $protect)
			$this->exception('File \'' . $file . '\' is not within the bounds of the protected scope: ' . $protect);
	}
}
