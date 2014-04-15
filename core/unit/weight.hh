<?hh  //  strict

/**
 *  Convert weight measurement units
 *  @name    CoreUnitLength
 *  @type    class
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 *  @author  Marco Balk <marco@uniqweb.nl>
 */
class CoreUnitWeight<Konsolidate> extends Konsolidate
{
	const STANDARD      = 'g';
	const GRAIN         = 0.06479891;
	const DRAM          = 1.7718451953125;
	const OUNCE         = 28.349523125;
	const POUND         = 453.59237;
	const HUNDREDWEIGHT = 45359.237;
	const SHORTTON      = 907184.74;
	const PENNYWEIGHT   = 1.55517384;
	const TROYOUNCE     = 31.1034768;
	const TROYPOUND     = 373.2417216;


	/**
	 *  Convert non-SI units to the base unit
	 *  @name    toBase
	 *  @type    method
	 *  @access  public
	 *  @param   string original value+unit
	 *  @param   bool   omit suffix
	 *  @return  mixed  value
	 */
	public function toBase(mixed $origin, bool $omitSuffix=false):string
	{
		$unit  = preg_replace('/[0-9\., -]*/', '', $origin);
		$value = floatVal($origin);
		$tmp   = $this->_convert($value, $unit);

		if (!is_null($tmp))
		{
			$unit  = '';
			$value = $tmp;
		}
		else if (substr($unit, -1) == self::STANDARD)
		{
			$unit = substr($unit, 0, -1);
		}
		
		return $this->call('../SI/prefixToBase', $value, $unit) . ($omitSuffix ? '' : self::STANDARD);
	}

	public function __call(string $unit, array $arg):?float
	{
		$unit   = str_replace('gram', '', strToLower($unit));
		$source = array_shift($arg);
		$tmp    = $this->_convert($this->toBase($source, true), $unit, -1);

		return !is_null($tmp) ? $tmp : $this->call('../SI/baseToPrefix', $this->toBase($source, true), $unit);
	}

	/**
	 *  Convert non-SI units to SI units (grams)
	 *  @name    load
	 *  @type    method
	 *  @access  protected
	 *  @param   number original value
	 *  @param   string original unit
	 *  @param   number conversion direction (from or to)
	 *  @return  number value
	 */
	protected function _convert(mixed $value, string $unit, int $direction=1):?float
	{
		$result = null;

		switch ($unit)
		{
			case 'grain':
			case 'gr':
				$result = $value * pow(self::GRAIN, $direction);
				break;

			case 'dram':
			case 'dr':
				$result = $value * pow(self::DRAM, $direction);
				break;

			case 'ounce':
			case 'oz':
				$result = $value * pow(self::OUNCE, $direction);
				break;

			case 'pound':
			case 'lb':
				$result = $value * pow(self::POUND, $direction);
				break;

			case 'hundredweight':
			case 'cwt':
				$result = $value * pow(self::HUNDREDWEIGHT, $direction);
				break;

			case 'ton':
			case 'shortton':
				$result = $value * pow(self::SHORTTON, $direction);
				break;

			case 'pennyweight':
			case 'dwt':
				$result = $value * pow(self::PENNYWEIGHT, $direction);
				break;

			case 'troyounce': 
			case 'ozt': 
			case 'oz t':
				$result = $value * pow(self::TROYOUNCE, $direction);
				break;

			case 'troypound': 
			case 'lbt': 
			case 'lb t':
				$result = $value * pow(self::TROYPOUND, $direction);
				break;
		}

		return !is_null($result) ? (float) $result : null;
	}
}
