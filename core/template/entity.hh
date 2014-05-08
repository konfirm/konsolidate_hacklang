<?hh  //  strict


/**
 *  Resolve HTML entities to a numeric (decimal or hexadecimal) representation or the actual referenced UTF-8 character
 *  @name    CoreTemplateEntity
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.net>
 */
class CoreTemplateEntity<Konsolidate> extends Konsolidate
{
	/**
	 *  Convert given entity to its UTF-8 character
	 *  @name   utf8
	 *  @type   method
	 *  @access public
	 *  @param  string entity
	 *  @return string UTF-8 character
	 */
	public function utf8(string $entity):string
	{
		return $this->call('UTF8/get', $this->_entityName($entity), $entity);
	}

	/**
	 *  Convert given entity to its decimal representation
	 *  @name   dec
	 *  @type   method
	 *  @access public
	 *  @param  string entity
	 *  @return string numeric (decimal) entity
	 */
	public function dec(string $entity):string
	{
		$dec = $this->numeric($entity);
		if (!empty($dec))
			return '&#' . $dec . ';';
		return $entity;
	}

	/**
	 *  Convert given entity to its hexadecimal representation
	 *  @name   hex
	 *  @type   method
	 *  @access public
	 *  @param  string entity
	 *  @return string numeric (hexadecimal) entity
	 */
	public function hex(string $entity):string
	{
		$dec = $this->numeric($entity);
		if (!empty($dec))
			return '&#x' . dechex($dec) . ';';
		return $entity;
	}

	/**
	 *  Obtain the number associated with the entity name
	 *  @name   numeric
	 *  @type   method
	 *  @access public
	 *  @param  string entity
	 *  @return number entity value
	 */
	public function numeric(string $entity):int
	{
		return $this->call('Numeric/get', $this->_entityName($entity));
	}

	/**
	 *  Get the real entity name (removing & and ;)
	 *  @name   _entityName
	 *  @type   method
	 *  @access protected
	 *  @param  string entity
	 *  @return string entity name
	 */
	protected function _entityName(string $entity):string
	{
		return preg_replace('/[^a-zA-Z]/', '', $entity);
	}
}
