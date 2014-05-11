<?hh  //  strict


/**
 *  MIME Detection/Guessing
 *  @name    CoreSystemFileMIME
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreSystemFileMIME<Konsolidate> extends Konsolidate
{
	/**
	 *  Try to determine the MIME type of a file
	 *  @name    getType
	 *  @type    method
	 *  @access  public
	 *  @param   string filename
	 *  @return  string MIME
	 *  @syntax  string CoreSystemFileMIME->getType(string filename)
	 */
	public function getType(string $file):string
	{
		$exists = file_exists($file);
		if ($exists && function_exists('finfo_open'))
			$method = '_determineTypeByFileInfo';
		else if ($exists && function_exists('mime_content_type'))
			$method = '_determineTypeByMimeContentType';
		else
			$method = '_determineTypeByExtension';


		return $this->{$method}($file);
	}

	/**
	 *  Try to determine the MIME type using 'mime_content_type'
	 *  @name    _determineTypeByMimeContentType
	 *  @type    method
	 *  @access  protected
	 *  @param   string filename
	 *  @return  string MIME
	 *  @syntax  string CoreSystemFileMIME->_determineTypeByMimeContentType(string filename)
	 */
	protected function _determineTypeByMimeContentType(string $file):string
	{
		return mime_content_type($file);
	}

	/**
	 *  Try to determine the MIME type using the fileinfo extension
	 *  @name    _determineTypeByFileInfo
	 *  @type    method
	 *  @access  protected
	 *  @param   string filename
	 *  @return  string MIME
	 *  @syntax  string CoreSystemFileMIME->_determineTypeByFileInfo(string filename)
	 */
	protected function _determineTypeByFileInfo(string $file):string
	{
		$finfo    = finfo_open(FILEINFO_MIME, $this->get('/Config/finfo_open/magic_file', null));
		$mimetype = finfo_file($finfo, $file);
		finfo_close($finfo);

		return $mimetype;
	}


	/**
	 *  Try to determine the MIME type using an (somewhat) educated guess based on the file extension
	 *  @name    _determineTypeByExtension
	 *  @type    method
	 *  @access  protected
	 *  @param   string filename
	 *  @returns string MIME
	 *  @syntax  string CoreSystemFileMIME->_determineTypeByExtension(string filename)
	 */
	protected function _determineTypeByExtension($file)
	{
		$part      = explode('.', $file);
	    $extension = array_pop($part);
	    $result    = 'application/octet-stream';

		switch (strToLower($extension))
		{
			//  Common image types
			case 'ai':
			case 'eps':
			case 'ps':
				$result = 'application/postscript';
				break;

			case 'bmp':
				$result = 'image/bmp';
				break;

			case 'gif':
				$result = 'image/gif';
				break;

			case 'jpe':
			case 'jpg':
			case 'jpeg':
				$result = 'image/jpeg';
				break;

			case 'png':
				$result = 'image/png';
				break;

			//  Common audio types
			case 'aifc':
			case 'aiff':
			case 'aif':
				$result = 'audio/aiff';
				break;

			case 'mid':
			case 'midi':
				$result = 'audio/midi';
				break;

			case 'mod':
				$result = 'audio/mod';
				break;

			case 'mp2':
				$result = 'audio/mpeg';
				break;

			case 'mp3':
				$result = 'audio/mpeg3';
				break;

			case 'wav':
				$result = 'audio/wav';
				break;

			//  Common video types
			case 'avi':
				$result = 'video/avi';
				break;

			case 'mov':
			case 'qt':
				$result = 'video/quicktime';
				break;

			case 'mpe':
			case 'mpg':
			case 'mpeg':
				$result = 'video/mpeg';
				break;

			//  Common text types
			case 'css':
				$result = 'text/css';
				break;

			case 'htm':
			case 'html':
			case 'htmls':
			case 'htx':
				$result = 'text/html';
				break;

			case 'conf':
			case 'log':
			case 'text':
			case 'txt':
			case 'php':
				$result = 'text/plain';
				break;

			case 'js':
				$result = 'application/x-javascript';
				break;

			case 'rtf':
				$result = 'text/richtext';
				break;

			case 'xml':
				$result = 'text/xml';
				break;

			case 'xsl':
			case 'xslt':
				$result = 'text/xslt';
				break;

			//  Other commonly used types
			case 'json':
				$result = 'application/json';
				break;

			case 'dcr':
				$result = 'application/x-director';
				break;

			case 'doc':
			case 'dot':
			case 'word':
				$result = 'application/msword';
				break;

			case 'gz':
			case 'gzip':
				$result = 'application/x-gzip';
				break;

			case 'latex':
				$result = 'application/x-latex';
				break;

			case 'pdf':
				$result = 'application/pdf';
				break;

			case 'pps':
			case 'ppt':
				$result = 'application/mspowerpoint';
				break;

			case 'swf':
				$result = 'application/x-shockwave-flash';
				break;

			case 'wp':
			case 'wp5':
			case 'wp6':
			case 'wpd':
				$result = 'application/wordperfect';
				break;

			case 'xls':
				$result = 'application/excel';
				break;

			case 'zip':
				$result = 'application/zip';
				break;
		}

		return $result;
	}
}
