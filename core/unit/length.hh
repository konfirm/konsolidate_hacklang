<?hh

/**
 *  Convert length measurement units
 *  @name    CoreUnitLength
 *  @type    class
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 *  @author  Marco Balk <marco@uniqweb.nl>
 */
class CoreUnitLength<Konsolidate> extends Konsolidate
{
	const STANDARD      = 'm';
	const INCH          = 0.0254;
	const FOOT          = 0.3048;
	const YARD          = 0.9144;
	const MILE          = 1609.344;
	const NAUTICAL_MILE = 1852;
	const KLICK         = 1000;

	/**
	 *  Convert non-SI units to the base unit
	 *  @name    toBase
	 *  @type    method
	 *  @access  public
	 *  @param   string original value+unit
	 *  @param   bool   append suffix
	 *  @return  mixed  value
	 */
	public function toBase(string $origin, $bOmitSuffix=false):string
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

		return $this->call( '../SI/prefixToBase', $value, $unit ) . ( $bOmitSuffix ? '' : self::STANDARD );
	}

	public function __call(string $unit, array $arg):?float
	{
		$unit   = str_replace('meter', '', $unit);
		$source = array_shift($arg);
		$tmp    = $this->_convert($this->toBase($source, true), $unit, -1);

		return !is_null($tmp) ? $tmp : $this->call( '../SI/baseToPrefix', $this->toBase($source, true), $unit);
	}

	/**
	 *  Convert non-SI units to SI units (meters)
	 *  @name    _convert
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
			case 'inch':
			case 'in':
				$result = $value * pow(self::INCH, $direction);
				break;

			case 'foot':
			case 'feet':
			case 'ft': 
				$result = $value * pow(self::FOOT, $direction);
				break;

			case 'yard':
			case 'yd':
				$result = $value * pow(self::YARD, $direction);
				break;

			case 'mile':
			case 'mi':
				$result = $value * pow(self::MILE, $direction);
				break;

			case 'nmi':
			case 'nm': 
				$result = $value * pow(self::NAUTICAL_MILE, $direction);
				break;

			case 'click':
			case 'klick':
				$result = $value * pow(self::KLICK, $direction);
				break;
		}

		return !is_null($result) ? (float) $result : null;
	}
}
