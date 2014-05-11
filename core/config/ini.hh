<?hh  //  strict


/**
 *  Read ini files and store ini-sections/variables for re-use in the Config Module
 *  @name    CoreConfigINI
 *  @type    class
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreConfigINI<Konsolidate> extends Konsolidate
{
	/**
	 *  Load and parse an inifile and store it's sections/variables in the Config Module
	 *  @name    load
	 *  @type    method
	 *  @access  public
	 *  @param   string  inifile
	 *  @param   string  section [optional, default null - all sections]
	 *  @return  array
	 */
	public function load(string $file, string $section=null):array<string, mixed>
	{
		$config = parse_ini_file($file, true);
		$result = Array();
		foreach ($config as $prefix=>$value)
		{
			if (is_array($value))
			{
				$result[$prefix] = array_key_exists('default', $result) ? $result['default'] : Array();
				foreach ($value as $key=>$val)
				{
					$result[$prefix][$key] = $val;
					$this->set('/Config/' . $prefix . '/' . $key, $val);
				}
			}
			else
			{
				$result[$prefix] = $value;
				$this->set('/Config/' . $prefix, $value);
			}
		}

		if (!is_null($section) && array_key_exists($section, $result))
			return $result[$section];

		return $result;
	}

	/**
	 *  Load and parse an inifile and create defines
	 *  @name    loadAndDefine
	 *  @type    method
	 *  @access  public
	 *  @param   string  inifile
	 *  @param   string  section [optional, default null - all sections]
	 *  @return  void
	 *  @note    defines are formatted like [SECTION]_[KEY]=[VALUE]
	 */
	public function loadAndDefine(string $file, string $section=null):void
	{
		$config = $this->load($file, $section);
		foreach ($config as $prefix=>$value)
			foreach ($value as $key=>$val)
			{
				$constant = strToUpper($prefix . '_' . $key);
				if (!defined($constant))
					define($constant, $value);
			}
	}
}