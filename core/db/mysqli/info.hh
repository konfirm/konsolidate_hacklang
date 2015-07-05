<?hh  //  strict


/**
 *  MySQL Info object
 *  @name    CoreDBMySQLiInfo
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreDBMySQLiInfo<Konsolidate> extends Konsolidate {
	/**
	 *  Process the information from a MySQLi instance
	 *  @name    process
	 *  @type    method
	 *  @access  public
	 *  @param   MySQLi   connection
	 *  @param   bool     extendedInfo
	 *  @param   Array    appendInfo
	 *  @return  void
	 */
	public function process(MySQLi $connection, bool $extendInfo=false, ?Array $appendInfo=null):void {
		$this->info = $connection->info;
		$info       = $this->_parseData($this->info);

		if (is_array($appendInfo)) {
			$info = array_merge($info, $appendInfo);
		}

		if ($extendInfo) {
			$this->stat = $connection->stat();
			$info = array_merge($info, $this->_parseData($this->stat));
		}

		foreach ($info as $key=>$value) {
			$this->{$key} = $value;
		}
	}

	/**
	 *  Collect specified or all information groups
	 *  @name    collect
	 *  @type    method
	 *  @access  public
	 *  @param   string  group1
	 *  @param   string  group...
	 *  @return  void
	 */
	public function collect():void {
		$arg = func_get_args();
		if (!count($arg)) {
			$arg = Array('Variable', 'Status', 'Table');
		}
		foreach ($arg as $module) {
			$this->register($module);
		}
	}

	/**
	 *  Parse a datablock containing information
	 *  @name    _parseData
	 *  @type    method
	 *  @access  protected
	 *  @param   string  data
	 *  @return  array
	 */
	protected function _parseData(?string $data):array {
		if (!empty($data) && (bool) preg_match_all('/([a-z_]+)\:([0-9\.]+),*/', preg_replace(Array('/\s\s/', '/\:\s/', '/\s/'), Array(',', ':', '_'), strtolower($data)), $match) && count($match) === 3) {
			return array_combine($match[1], $match[2]);
		}

		return Array();
	}

	/**
	 *  Automatically populate child modules (Variables, Table and Status if requested)
	 *  @name    register
	 *  @type    method
	 *  @access  public
	 *  @param   string module/connection
	 *  @returns Object
	 *  @note    this method is an override to Konsolidates default behaviour
	 */
	public function register(string $module):object {
		$instance = parent::register($module);
		switch (strtolower($module)) {
			case 'variable':
				$result = $this->call('../query', 'SHOW VARIABLES');
				if (is_object($result) && $result->errno <= 0 && (bool) $result->rows)
					while ($record = $result->next())
						$instance->set(strtolower($record->Variable_name), $record->Value);
				break;

			case 'status':
				$result = $this->call('../query', 'SHOW GLOBAL STATUS');
				if (is_object( $result ) && $result->errno <= 0 && (bool) $result->rows)
					while ($record = $result->next())
						$instance->set('Global/' . strtolower($record->Variable_name), $record->Value);

				$result = $this->call('../query', 'SHOW SESSION STATUS');
				if (is_object($result) && $result->errno <= 0 && (bool) $result->rows)
					while ($record = $result->next())
						$instance->set('Session/' . strtolower($record->Variable_name), $record->Value);
				break;

			case 'table':
				$result = $this->call('../query', 'SHOW TABLE STATUS');
				if (is_object($result) && $result->errno <= 0 && (bool) $result->rows)
					while ($record = $result->next()) {
						$sName = $record->Name;
						foreach( $record as $sKey=>$sValue )
							$instance->set($sName . '/' . strToLower($sKey ), $sValue);
					}
				break;
		}

		return $instance;
	}
}
