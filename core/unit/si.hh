<?hh  //  strict


/**
 *  Convert SI measurement units
 *  @name    CoreUnitSI
 *  @type    class
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 *  @author  Marco Balk <marco@uniqweb.nl>
 */
class CoreUnitSI<Konsolidate> extends Konsolidate
{
	const YOTA     = 24;     const DECI  = -1;
	const ZETTA    = 21;     const CENTI = -2;
	const EXA      = 18;     const MILLI = -3;
	const PETA     = 15;     const MICRO = -6;
	const TERA     = 12;     const NANO  = -9;
	const GIGA     = 9;      const PICO  = -12;
	const MEGA     = 6;      const FEMTO = -15;
	const KILO     = 3;      const ATTO  = -18;
	const HECTO    = 2;      const ZEPTO = -21;
	const DECA     = 1;      const YOCTO = -24;
	const STANDARD = 0;

	protected array<string, string> $_unitMatrix = Array('YOTA'=>'Y', 'ZETTA'=>'Z', 'EXA'=>'E', 'PETA'=>'P', 'TERA'=>'T', 'GIGA'=>'G', 'MEGA'=>'M', 'KILO'=>'k', 'HECTO'=>'h', 'DECA'=>'da', 'DECI'=>'d', 'CENTI'=>'c', 'MILLI'=>'m', 'MICRO'=>'μ', 'NANO'=>'n', 'PICO'=>'p', 'FEMTO'=>'f', 'ATTO'=>'a', 'ZEPTO'=>'z', 'YOCTO'=>'y', 'STANDARD'=>'');

	
	/**
	 *  Convert base units to the non-SI unit
	 *  @name    baseToPrefix
	 *  @type    method
	 *  @access  public
	 *  @param   string original value+unit
	 *  @param   string original unit
	 *  @return  mixed  value
	 */
	public function baseToPrefix(mixed $base, string $fix=''):float
	{
		return $this->_convert($base, $fix);
	}
	
	/**
	 *  Convert non-SI units to the base unit
	 *  @name    prefixToBase
	 *  @type    method
	 *  @access  public
	 *  @param   string original value+unit
	 *  @param   string original unit
	 *  @return  mixed  value
	 */
	public function prefixToBase(mixed $base, string $fix=''):float
	{
		return $this->_convert($base, $fix, -1);
	}

	/**
	 *  Convert SI units to non-SI units
	 *  @name    _convert
	 *  @type    method
	 *  @access  protected
	 *  @param   number original value
	 *  @param   string original unit
	 *  @param   number conversion direction (from or to)
	 *  @return  number value
	 */
	protected function _convert(mixed $base, string $fix='', int $direction=1):float
	{
		if (!array_key_exists(strToUpper($fix), $this->_unitMatrix))
			$fix = array_search($fix, $this->_unitMatrix);
		else
			$fix = strToUpper($fix);

		if ($fix === false)
			throw new Exception('No or unknown pre-/suffix provided');
		
		return (float) ($base / pow(10, ($direction * constant('static::' . $fix))));
	}
}
