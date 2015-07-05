<?hh  //  strict

define('TINYINT_MIN', -128);        // for future reference
define('TINYINT_MAX',  127);        // for future reference
define('SMALLINT_MIN', -32768);     // for future reference
define('SMALLINT_MAX',  32767);     // for future reference
define('MEDIUMINT_MIN', -8388608);  // for future reference
define('MEDIUMINT_MAX',  8388607);  // for future reference
define('INT_MIN', -2147483648);
define('INT_MAX', 2147483647);


/**
 *  Basic validation
 *  @name    CoreValidate
 *  @type    class
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreValidate<Konsolidate> extends Konsolidate {
	/**
	 *  is the value an integer
	 *  @name    isInteger
	 *  @type    method
	 *  @access  public
	 *  @param   mixed value
	 *  @param   bool  unsigned [optional]
	 *  @return  bool
	 */
	function isInteger(mixed $value, bool $unsigned=false):bool {
		$min = $unsigned ? 0 : INT_MIN;
		$max = $unsigned ? INT_MAX + (-INT_MIN) : INT_MAX;

		if (is_null($value) || (!preg_match('/^[0-9]+$/', abs($value))) || $value < $min || $value > $max) {
			return false;
		}

		return true;
	}

	/**
	 *  is the value a positive integer
	 *  @name    isPositiveInteger
	 *  @type    method
	 *  @access  public
	 *  @param   mixed value
	 *  @param   bool  unsigned [optional]
	 *  @return  bool
	 */
	function isPositiveInteger(mixed $value, bool $unsigned=false):bool {
		return ( $this->isInteger($value, $unsigned) && $value >= 0);
	}

	/**
	 *  is the value a negative integer
	 *  @name    isNegativeInteger
	 *  @type    method
	 *  @access  public
	 *  @param   mixed value
	 *  @return  bool
	 */
	function isNegativeInteger(mixed $value):bool {
		return $this->isInteger($value) && $value < 0;
	}

	/**
	 *  is the value a number
	 *  @name    isNumber
	 *  @type    method
	 *  @access  public
	 *  @param   mixed value
	 *  @return  bool
	 */
	function isNumber(mixed $value):bool {
		return is_numeric($value);
	}

	/**
	 *  is the value between (or equal to) two values
	 *  @name    isBetween
	 *  @type    method
	 *  @access  public
	 *  @param   mixed value
	 *  @param   int   minimum [optional]
	 *  @param   int   maximum [optional]
	 *  @param   bool  include min/max values [optional]
	 *  @return  bool
	 */
	function isBetween(mixed $value, int $min=null, int $max=null, bool $inclusive=true):bool {
		$result = false;

		if ($inclusive) {
			if (!is_null($min)) {
				$min -= 1;
			}

			if (!is_null($max)) {
				$max += 1;
			}
		}

		if (!is_null($min) && !is_null($max)) {
			$result = $value > $min && $value < $max;
		}
		else if (!is_null($min)) {
			$result = $value > $min;
		}
		else if (!is_null($max)) {
			$result = $value < $max;
		}

		return $result;
	}

	/**
	 *  does the variable contain a value
	 *  @name    isFilled
	 *  @type    method
	 *  @access  public
	 *  @param   mixed value
	 *  @return  bool
	 */
	function isFilled(mixed $value):bool {
		return !preg_match('/^$/', $value);
	}

	/**
	 *  does the value represent a possible e-mail address
	 *  @name    isEmail
	 *  @type    method
	 *  @access  public
	 *  @param   mixed value
	 *  @return  bool
	 *  @note    This method does NOT verify the actual existing of the e-mail address, it merely verifies that it complies to common e-mail addresses
	 */
	function isEmail($value) {
		return preg_match('/^[_a-z0-9-]+([a-z0-9\.\+_-]+)*@[a-z0-9-]+(\.[a-z0-9-]+)*(\.[a-z]{2,3}|.info)$/i', $value);
	}
}
