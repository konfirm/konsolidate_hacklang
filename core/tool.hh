<?hh

/**
 *  Abstracted functionality often used in- and outside Konsolidate
 *  @name    CoreTool
 *  @type    class
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 *  @note    This class is always available as soon as Konsolidate (and its extends) is instanced (and the Core tier is available)
 */
class CoreTool<Konsolidate> extends Konsolidate
{
	/**
	 *  Determine whether the script is access from a POST request
	 *  @name    isPosted
	 *  @type    method
	 *  @access  public
	 *  @return  bool
	 */
	public static function isPosted():bool
	{
		return isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] === 'POST';
	}

	/**
	 *  Get a value from an array
	 *  @name    arrVal
	 *  @type    method
	 *  @access  public
	 *  @param   string key
	 *  @param   array  collection
	 *  @param   mixed  default [optional, default null]
	 *  @return  mixed
	 *  @note    you can swap key and collection (Haystack - Needle)
	 */
	public static function arrVal(string $key, array<string, mixed> $collection, ?mixed $default):mixed
	{
		if (!is_array($collection) && is_array($key))
			return CoreTool::arrVal($collection, $key, $default);

		return is_array($collection ) && isset($collection[$key]) ? $collection[$key] : $default;
	}

	/**
	 *  Alias for arrVal
	 *  @name    arrayVal
	 *  @type    method
	 *  @access  public
	 *  @param   string key
	 *  @param   array  collection
	 *  @param   mixed  default [optional, default null]
	 *  @return  mixed
	 *  @see     arrVal
	 */
	public static function arrayVal(string $key, array<string, mixed> $collection, ?mixed $default):mixed
	{
		return CoreTool::arrVal($key, $collection, $default);
	}
	
	/**
	 *  Get a value from a PHP Session (not a CoreSession!)
	 *  @name    sesVal
	 *  @type    method
	 *  @access  public
	 *  @param   string key
	 *  @param   mixed  default [optional, default null]
	 *  @return  mixed
	 *  @note    This method works with PHP's built in sessions (_SESSION global), not CoreSession!
	 */
	public static function sesVal(string $key, ?mixed $default):mixed
	{
		return CoreTool::arrVal($key, $_SESSION, $default);
	}

	/**
	 *  Alias for sesVal
	 *  @name    sessionVal
	 *  @type    method
	 *  @access  public
	 *  @param   string key
	 *  @param   mixed  default [optional, default null]
	 *  @return  mixed
	 *  @see     sesVal
	 */
	public static function sessionVal(string $key, ?mixed $default):mixed
	{
		return CoreTool::sesVal($key, $default);
	}

	/**
	 *  Get a value from a PHP Cookie
	 *  @name    cookieVal
	 *  @type    method
	 *  @access  public
	 *  @param   string key
	 *  @param   mixed  default [optional, default null]
	 *  @return  mixed
	 */
	public static function cookieVal(string $key, ?mixed $default):mixed
	{
		return CoreTool::arrVal($key, $_COOKIE, $default);
	}

	/**
	 *  Get a value from the _SERVER global
	 *  @name    serverVal
	 *  @type    method
	 *  @access  public
	 *  @param   string key
	 *  @param   mixed  default [optional, default null]
	 *  @return  mixed
	 */
	public static function serverVal(string $key, $default=null):mixed
	{
		return CoreTool::arrVal($key, $_SERVER, $default);
	}

	/**
	 *  Get a value from the environment
	 *  @name    envVal
	 *  @type    method
	 *  @access  public
	 *  @param   string key
	 *  @param   mixed  default [optional, default null]
	 *  @return  mixed
	 */
	public static function envVal(string $key, $default=null):mixed
	{
		return CoreTool::arrVal($key, $_ENV, $default);
	}

	/**
	 *  Alias for envVal
	 *  @name    environmentVal
	 *  @type    method
	 *  @access  public
	 *  @param   string key
	 *  @param   mixed  default [optional, default null]
	 *  @return  mixed
	 *  @see     envVal
	 */
	public static function environmentVal(string $key, ?mixed $default):mixed
	{
		return CoreTool::envVal($key, $default);
	}

	/**
	 *  Get the User/Visitor IP address
	 *  @name    getIP
	 *  @type    method
	 *  @access  public
	 *  @param   mixed  default [optional, default null]
	 *  @return  mixed
	 */
	public static function getIP(string $default='0.0.0.0'):string
	{
		if ($return = CoreTool::serverVal('HTTP_CLIENT_IP'))
			return $return;
		if ($return = CoreTool::serverVal('HTTP_X_FORWARDED_FOR'))
			return $return;
		if ($return = CoreTool::serverVal('REMOTE_ADDR'))
			return $return;
		return $default;
	}

	/**
	 *  Redirect the browser to another location
	 *  @name    redirect
	 *  @type    method
	 *  @access  public
	 *  @param   string URL
	 *  @param   bool   stopscript
	 *  @return  void
	 *  @note    this method sends out both a META header and a JavaScript in case headers were already sent
	 */
	public static function redirect(string $URL, bool $die=true):void
	{
		if (!headers_sent())
			header('Location: ' . $URL);
		else
			print('<meta http-equiv="refresh" content="0;URL=' . $URL . '"></meta>\n<script type="text/javascript">location.href="' . $URL . '";</script>');
		
		if ($die)
			exit;
	}

	/**
	 *  Expire page by sending out a variaty of expiration headers
	 *  @name    expirePage
	 *  @type    method
	 *  @access  public
	 *  @param   int    timestamp [optional, default 946702800]
	 *  @return  bool
	 */
	public static function expirePage(?int $timestamp):bool
	{
		if (!headers_sent())
		{
			if (is_null($timestamp))
				$timestamp = 946702800;
			header('Cache-Control: no-cache, must-revalidate, private');
			header('Pragma: no-cache');
			header('Expires: ' . gmdate('D, d M Y H:i:s', $timestamp ) . ' GMT');
			header('Last-Modified: ' . gmdate('D, d M Y H:i:s') . ' GMT');
			return true;
		}
		return false;
	}
}