<?hh  //  strict


/**
 *  Group together features returned by CoreTemplate::getFeatures and bridge all template calls to each member of the group
 *  @name    CoreTemplateGroup
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreTemplateGroup<Konsolidate> extends Konsolidate {
	protected Vector<CoreTemplateFeature> $_group;


	/**
	 *  Constructor
	 *  @name   __construct
	 *  @type   method
	 *  @access public
	 *  @param  Konsolidate $parent
	 *  @param  Array feature group
	 *  @return CoreTemplateGroup object
	 */
	public function __construct(Konsolidate $parent, Vector<CoreTemplateFeature> $group=null) {
		parent::__construct($parent);

		$this->_group = $group;
	}

	/**
	 *  Magic __call method, bridging all undeclared methods to each feature in the group
	 *  @name   __call
	 *  @type   method
	 *  @access public
	 *  @param  string method
	 *  @param  mixed argument
	 *  @return void
	 */
	public function __call(string $method, Array<string, mixed> $argument):void {
		foreach ($this->_group as $member) {
			call_user_func_array(Array($member, $method), $argument);
		}
	}

	/**
	 *  Implement the template's block method and return a new group
	 *  @name   block
	 *  @type   method
	 *  @access public
	 *  @param  string name
	 *  @return CoreTemplateGroup object
	 */
	public function block(string $name):CoreTemplateGroup {
		$return = Array();
		foreach ($this->_group as $member) {
			$return[] = $member->block($name);
		}

		return count($return) > 0 ? $this->instance('../Group', $return) : false;
	}

	/**
	 *  Magic setter, setting the given property to each feature in the group
	 *  @name   __set
	 *  @type   method
	 *  @access public
	 *  @param  string property
	 *  @param  mixed value
	 *  @return void
	 */
	public function __set(string $property, mixed $value):void {
		foreach ($this->_group as $member) {
			$member->{$property} = $value;
		}
	}


	/**
	 *  Magic getter, returning the first occurrence of the requirested property
	 *  @name   __get
	 *  @type   method
	 *  @access public
	 *  @param  string property
	 *  @return mixed value
	 */
	public function __get(string $property):mixed {
		foreach ($this->_group as $member) {
			if (isset($member->{$property})) {
				return $member->{$property};
			}
		}
	}
}
