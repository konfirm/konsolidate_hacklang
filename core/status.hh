<?hh  //  strict


/**
 *  Send status headers and keep track of the last one sent
 *  @name    CoreStatus
 *  @package Konsolidate
 *  @author  Rogier Spieker <rogier@konsolidate.nl>
 */
class CoreStatus<Konsolidate> extends Konsolidate {
	/**
	 *  Send the status header
	 *  @name    send
	 *  @type    method
	 *  @access  public
	 *  @param   int     status code
	 *  @param   string  message [optional, default null - the message associated with the status code]
	 *  @param   bool    use status header [optional, default false - use HTTP/<version>]
	 *  @param   string  protocolVersion [optional, default 'all' - all possible HTTP status codes
	 *                                    valid values: (HTTP/)1.0, (HTTP/)1.1, (WEB)DAV. 'all' is assumed otherwise]
	 *  @return  bool    success
	 *  @note    if the headers are already sent, no attempt to send new status headers is made and return return
	 *           value is false.
	 */
	public function send(int $status, string $message=null, bool $useStatus=false, ?string $protocolVersion='all'):bool {
		if (headers_sent())
			return false;

		header(sprintf(
			'%s %d %s',
			$useStatus ? 'Status:' : $this->_getProtocol(),
			$status,
			$message ?: $this->getMessage($status, $protocolVersion ?: $this->_getProtocolVersion())
		), true, $status);

		//  keep track of the current status
		$this->current = $status;

		return true;
	}

	/**
	 *  Obtain the status message associated with the status code
	 *  @name    getMessage
	 *  @type    method
	 *  @access  public
	 *  @param   int   status code
	 *  @return  string message
	 */
	public function getMessage(int $status, ?string $protocol=null):string {
		$type = $this->_getProtocolDefinitions($protocol);

		return $type->get($status) ?: 'Unknown';
	}

	/**
	 *  Obtain the protocol used for the current request, defaults to 'HTTP/1.0' if it could not be determined
	 *  @name    _getProtocol
	 *  @type    method
	 *  @access  protected
	 *  @return  string  protocol
	 */
	protected function _getProtocol():string {
		return $this->call('/Tool/serverVal', 'SERVER_PROTOCOL', 'HTTP/1.0');
	}

	/**
	 *  Determine the protocol version set in the protocol argument, if no argument is provided the current request
	 *  protocol will be used
	 *  @name    _getProtocolVersion
	 *  @type    method
	 *  @access  protected
	 *  @param   string  protocol [optional, default null - the request protocol]
	 *  @return  string  version
	 */
	protected function _getProtocolVersion(?string $protocol=null):string {
		$result = '1.0';

		if (preg_match('/\b([0-9\.]+)$/i', $protocol ?: $this->getProtocol(), $match)) {
			$result = $match[1];
		}

		return $result;
	}

	/**
	 *  Obtain all valid status code/message pairs based on the given protocol
	 *  @name    _getProtocolDefinitions
	 *  @type    method
	 *  @access  protected
	 *  @param   string  protocol [optional, default 'all' - combine all definitions]
	 *  @param   bool    sort     [optional, default false - no sort]
	 *  @return  Map<int code, string message>
	 */
	protected function _getProtocolDefinitions(?string $protocol='all', ?bool $sort=false):ImmMap<int, string> {
		//  Hypertext Transfer Protocol - HTTP/1.0
		//  http://tools.ietf.org/html/1945
		static $rfc1945 = ImmMap<int, string> {
			/**  2xx  -  Successful  **/

			//  The request has succeeded.
			200 => 'OK',

			//  The request has been fulfilled and resulted in a new resource being created. The newly created
			//  resource can be referenced by the URI(s) returned in the entity of the response. The origin server
			//  should create the resource before using this Status-Code. If the action cannot be carried out
			//  immediately, the server must include in the response body a description of when the resource will be
			//  available; otherwise, the server should respond with 202 (accepted). Of the methods defined by this
			//  specification, only POST can create a resource.
			201 => 'Created',

			//  The request has been accepted for processing, but the processing has not been completed. The request
			//  may or may not eventually be acted upon, as it may be disallowed when processing actually takes
			//  place. There is no facility for re-sending a status code from an asynchronous operation such
			//  as this.
			202 => 'Accepted',

			//  The server has fulfilled the request but there is no new information to send back. If the client is
			//  a user agent, it should not change its document view from that which caused the request to be
			//  generated. This response is primarily intended to allow input for scripts or other actions to take
			//  place without causing a change to the user agent's active document view. The response may include
			//  new metainformation in the form of entity headers, which should apply to the document currently in
			//  the user agent's active view.
			204 => 'No Content',

			/**  3xx  -  Redirection  **/

			//  This response code is not directly used by HTTP/1.0 applications, but serves as the default for
			//  interpreting the 3xx class of responses. The requested resource is available at one or more
			//  locations. Unless it was a HEAD request, the response should include an entity containing a list of
			//  resource characteristics and locations from which the user or user agent can choose the one most
			//  appropriate. If the server has a preferred choice, it should include the URL in a Location field;
			//  user agents may use this field value for automatic redirection.
			300 => 'Multiple Choices',

			//  The requested resource has been assigned a new permanent URL and any future references to this
			//  resource should be done using that URL. Clients with link editing capabilities should automatically
			//  relink references to the Request-URI to the new reference returned by the server, where possible.
			//  The new URL must be given by the Location field in the response. Unless it was a HEAD request, the
			//  Entity-Body of the response should contain a short note with a hyperlink to the new URL. If the 301
			//  status code is received in response to a request using the POST method, the user agent must not
			//  automatically redirect the request unless it can be confirmed by the user, since this might change
			//  the conditions under which the request was issued.
			301 => 'Moved Permanently',

			//  The requested resource resides temporarily under a different URL. Since the redirection may be
			//  altered on occasion, the client should continue to use the Request-URI for future requests. The URL
			//  must be given by the Location field in the response. Unless it was a HEAD request, the Entity-Body
			//  of the response should contain a short note with a hyperlink to the new URI(s). If the 302 status
			//  code is received in response to a request using the POST method, the user agent must not
			//  automatically redirect the request unless it can be confirmed by the user, since this might change
			//  the conditions under which the request was issued.
			302 => 'Moved Temporarily',

			//  If the client has performed a conditional GET request and access is allowed, but the document has
			//  not been modified since the date and time specified in the If-Modified-Since field, the server must
			//  respond with this status code and not send an Entity-Body to the client. Header fields contained in
			//  the response should only include information which is relevant to cache managers or which may have
			//  changed independently of the entity's Last-Modified date. Examples of relevant header fields
			//  include: Date, Server, and Expires. A cache should update its cached entity to reflect any new field
			//  values given in the 304 response.
			304 => 'Not Modified',

			/**  4xx  -  User Error  **/

			//  The request could not be understood by the server due to malformed syntax. The client should not
			//  repeat the request without modifications.
			400 => 'Bad Request',

			//  The request requires user authentication. The response must include a WWW-Authenticate header field
			//  containing a challenge applicable to the requested resource. The client may repeat the request with
			//  a suitable Authorization header field. If the request already included Authorization credentials,
			//  then the 401 response indicates that authorization has been refused for those credentials. If the
			//  401 response contains the same challenge as the prior response, and the user agent has already
			//  attempted authentication at least once, then the user should be presented the entity that was given
			//  in the response, since that entity may include relevant diagnostic information.
			401 => 'Unauthorized',

			//  The server understood the request, but is refusing to fulfill it. Authorization will not help and
			//  the request should not be repeated. If the request method was not HEAD and the server wishes to make
			//  public why the request has not been fulfilled, it should describe the reason for the refusal in the
			//  entity body. This status code is commonly used when the server does not wish to reveal exactly why
			//  the request has been refused, or when no other response is applicable.
			403 => 'Forbidden',

			//  The server has not found anything matching the Request-URI. No indication is given of whether the
			//  condition is temporary or permanent. If the server does not wish to make this information available
			//  to the client, the status code 403 (forbidden) can be used instead.
			404 => 'Not Found',

			/**  5xx  -  Server Error  **/

			//  The server encountered an unexpected condition which prevented it from fulfilling the
			//  request.
			500 => 'Internal Server Error',

			//  The server does not support the functionality required to fulfill the request. This is the
			//  appropriate response when the server does not recognize the request method and is not capable of
			//  supporting it for any resource.
			501 => 'Not Implemented',

			//  The server, while acting as a gateway or proxy, received an invalid response from the upstream
			//  server it accessed in attempting to fulfill the request.
			502 => 'Bad Gateway',

			//  The server is currently unable to handle the request due to a temporary overloading or maintenance
			//  of the server. The implication is that this is a temporary condition which will be alleviated after
			//  some delay.
			503 => 'Service Unavailable'
		};

		//  Hypertext Transfer Protocol - HTTP/1.1
		//  http://tools.ietf.org/html/2068
		static $rfc2068 = ImmMap<int, string> {
			/**  1xx  -  Informational  **/

			//  The client may continue with its request. This interim response is used to inform the client that
			//  the initial part of the request has been received and has not yet been rejected by the server. The
			//  client SHOULD continue by sending the remainder of the request or, if the request has already been
			//  completed, ignore this response. The server MUST send a final response after the request has
			//  been completed.
			100 => 'Continue',

			//  The server understands and is willing to comply with the client's request, via the Upgrade message
			//  header field, for a change in the application protocol being used on this connection. The server
			//  will switch protocols to those defined by the response's Upgrade header field immediately after the
			//  empty line which terminates the 101 response.
			101 => 'Switching Protocols',

			/**  2xx  -  Successful  **/

			//  The returned metainformation in the entity-header is not the definitive set as available from the
			//  origin server, but is gathered from a local or a third-party copy. The set presented MAY be a subset
			//  or superset of the original version. For example, including local annotation information about the
			//  resource MAY result in a superset of the metainformation known by the origin server. Use of this
			//  response code is not required and is only appropriate when the response would otherwise be
			//  200 (OK).
			203 => 'Non-Authoritative Information',

			//  The server has fulfilled the request and the user agent SHOULD reset the document view which caused
			//  the request to be sent. This response is primarily intended to allow input for actions to take place
			//  via user input, followed by a clearing of the form in which the input is given so that the user can
			//  easily initiate another input action. The response MUST NOT include an entity.
			205 => 'Reset Content',

			//  The server has fulfilled the partial GET request for the resource. The request must have included a
			//  Range header field indicating the desired range. The response MUST include either a Content-Range
			//  header field indicating the range included with this response, or a multipart/byteranges
			//  Content-Type including Content-Range fields for each part. If multipart/byteranges is not used, the
			//  Content-Length header field in the response MUST match the actual number of OCTETs transmitted in
			//  the message-body. A cache that does not support the Range and Content-Range headers MUST NOT cache
			//  206 (Partial) responses.
			206 => 'Partial Content',

			/**  3xx  -  Redirection  **/

			//  The response to the request can be found under a different URI and SHOULD be retrieved using a GET
			//  method on that resource. This method exists primarily to allow the output of a POST-activated script
			//  to redirect the user agent to a selected resource. The new URI is not a substitute reference for the
			//  originally requested resource. The 303 response is not cachable, but the response to the second
			//  (redirected) request MAY be cachable. If the new URI is a location, its URL SHOULD be given by the
			//  Location field in the response. Unless the request method was HEAD, the entity of the response
			//  SHOULD contain a short hypertext note with a hyperlink to the new URI(s).
			303 => 'See Other',

			//  The requested resource MUST be accessed through the proxy given by the Location field. The Location
			//  field gives the URL of the proxy. The recipient is expected to repeat the request via the
			//  proxy.
			305 => 'Use Proxy',

			/**  4xx  -  User Error  **/

			//  This code is reserved for future use.
			402 => 'Payment Required',

			//  The method specified in the Request-Line is not allowed for the resource identified by the
			//  Request-URI. The response MUST include an Allow header containing a list of valid methods for the
			//  requested resource.
			405 => 'Method Not Allowed',

			//  The resource identified by the request is only capable of generating response entities which have
			//  content characteristics not acceptable according to the accept headers sent in the request.
			406 => 'Not Acceptable',

			//  This code is similar to 401 (Unauthorized), but indicates that the client MUST first authenticate
			//  itself with the proxy. The proxy MUST return a Proxy-Authenticate header field containing a
			//  challenge applicable to the proxy for the requested resource. The client MAY repeat the request with
			//  a suitable Proxy-Authorization header field.
			407 => 'Proxy Authentication Required',

			//  The client did not produce a request within the time that the server was prepared to wait. The
			//  client MAY repeat the request without modifications at any later time.
			408 => 'Request Timeout',

			//  The request could not be completed due to a conflict with the current state of the resource. This
			//  code is only allowed in situations where it is expected that the user might be able to resolve the
			//  conflict and resubmit the request. The response body SHOULD include enough information for the user
			//  to recognize the source of the conflict. Ideally, the response entity would include enough
			//  information for the user or user agent to fix the problem; however, that may not be possible and is
			//  not required. Conflicts are most likely to occur in response to a PUT request. If versioning is
			//  being used and the entity being PUT includes changes to a resource which conflict with those made by
			//  an earlier (third-party) request, the server MAY use the 409 response to indicate that it can't
			//  complete the request. In this case, the response entity SHOULD contain a list of the differences
			//  between the two versions in a format defined by the response Content-Type.
			409 => 'Conflict',

			//  The requested resource is no longer available at the server and no forwarding address is known. This
			//  condition SHOULD be considered permanent. Clients with link editing capabilities SHOULD delete
			//  references to the Request-URI after user approval. If the server does not know, or has no facility
			//  to determine, whether or not the condition is permanent, the status code 404 (Not Found) SHOULD be
			//  used instead. This response is cachable unless indicated otherwise. The 410 response is primarily
			//  intended to assist the task of web maintenance by notifying the recipient that the resource is
			//  intentionally unavailable and that the server owners desire that remote links to that resource be
			//  removed. Such an event is common for limited-time, promotional services and for resources belonging
			//  to individuals no longer working at the server's site. It is not necessary to mark all permanently
			//  unavailable resources as 'gone' or to keep the mark for any length of time.
			410 => 'Gone',

			//  The server refuses to accept the request without a defined Content- Length. The client MAY repeat
			//  the request if it adds a valid Content-Length header field containing the length of the message-body
			//  in the request message.
			411 => 'Length Required',

			//  The precondition given in one or more of the request-header fields evaluated to false when it was
			//  tested on the server. This response code allows the client to place preconditions on the current
			//  resource metainformation (header field data) and thus prevent the requested method from being
			//  applied to a resource other than the one intended.
			412 => 'Precondition Failed',

			//  The server is refusing to process a request because the request entity is larger than the server is
			//  willing or able to process. The server may close the connection to prevent the client from
			//  continuing the request. If the condition is temporary, the server SHOULD include a Retry-After
			//  header field to indicate that it is temporary and after what time the client may try again.
			413 => 'Request Entity Too Large',

			//  The server is refusing to service the request because the Request-URI is longer than the server is
			//  willing to interpret. This rare condition is only likely to occur when a client has improperly
			//  converted a POST request to a GET request with long query information, when the client has descended
			//  into a URL 'black hole' of redirection (e.g., a redirected URL prefix that points to a suffix of
			//  itself), or when the server is under attack by a client attempting to exploit security holes present
			//  in some servers using fixed-length buffers for reading or manipulating the Request-URI.
			414 => 'Request-URI Too Long',

			//  The server is refusing to service the request because the entity of the request is in a format not
			//  supported by the requested resource for the requested method.
			415 => 'Unsupported Media Type',

			/**  5xx  -  Server Error  **/

			//  The server, while acting as a gateway or proxy, did not receive a timely response from the upstream
			//  server it accessed in attempting to complete the request.
			504 => 'Gateway Timeout',

			//  The server does not support, or refuses to support, the HTTP protocol version that was used in the
			//  request message. The server is indicating that it is unable or unwilling to complete the request
			//  using the same major version as the client, other than with this error message. The response SHOULD
			//  contain an entity describing why that version is not supported and what other protocols are
			//  supported by that server.
			505 => 'HTTP Version Not Supported'
		};

		//  Transparent Content Negotiation in HTTP
		//  http://tools.ietf.org/html/2295
		static $rfc2295 = ImmMap<int, string> {
			/**  5xx  -  Server Error  **/

			//  The 506 status code indicates that the server has an internal configuration error: the chosen
			//  variant resource is configured to engage in transparent content negotiation itself, and is therefore
			//  not a proper end point in the negotiation process.
			506 => 'Variant Also Negotiates'
		};

		//  Hyper Text Coffee Pot Control Protocol (HTCPCP/1.0)
		//  http://tools.ietf.org/html/2324
		static $rfc2324 = ImmMap<int, string> {
			/**  4xx  -  User Error  **/

			//  Any attempt to brew coffee with a teapot should result in the error code "418 I'm a teapot". The
			//  resulting entity body MAY be short and stout.
			418 => 'I\'m A Teapot'
		};

		//  HTTP Extensions for Distributed Authoring - WEBDAV
		//  http://tools.ietf.org/html/2518
		static $rfc2518 = ImmMap<int, string> {
			/**  1xx  -  Informational  **/

			//  The 102 (Processing) status code is an interim response used to inform the client that the server
			//  has accepted the complete request, but has not yet completed it. This status code SHOULD only be
			//  sent when the server has a reasonable expectation that the request will take significant time to
			//  complete. As guidance, if a method is taking longer than 20 seconds (a reasonable, but arbitrary
			//  value) to process the server SHOULD return a 102 (Processing) response. The server MUST send a final
			//  response after the request has been completed. Methods can potentially take a long period of time to
			//  process, especially methods that support the Depth header. In such cases the client may time-out the
			//  connection while waiting for a response. To prevent this the server may return a 102 (Processing)
			//  status code to indicate to the client that the server is still processing the method.
			102 => 'Processing',

			/**  2xx  -  Successful  **/

			//  The 207 (Multi-Status) status code provides status for multiple independent operations.
			207 => 'Multi-Status',

			/**  4xx  -  User Error  **/

			//  The 422 (Unprocessable Entity) status code means the server understands the content type of the
			//  request entity (hence a 415 (Unsupported Media Type) status code is inappropriate), and the syntax
			//  of the request entity is correct (thus a 400 (Bad Request) status code is inappropriate) but was
			//  unable to process the contained instructions. For example, this error condition may occur if an XML
			//  request body contains well-formed (i.e., syntactically correct), but semantically erroneous
			//  XML instructions.
			422 => 'Unprocessable Entity',

			//  The 423 (Locked) status code means the source or destination resource of a method is locked.
			423 => 'Locked',

			//  The 424 (Failed Dependency) status code means that the method could not be performed on the resource
			//  because the requested action depended on another action and that action failed. For example, if a
			//  command in a PROPPATCH method fails then, at minimum, the rest of the commands will also fail with
			//  424 (Failed Dependency).
			424 => 'Failed Dependency',

			/**  5xx  -  Server Error  **/

			//  The 507 (Insufficient Storage) status code means the method could not be performed on the resource
			//  because the server is unable to store the representation needed to successfully complete the
			//  request. This condition is considered to be temporary. If the request which received this status
			//  code was the result of a user action, the request MUST NOT be repeated until it is requested by a
			//  separate user action.
			507 => 'Insufficient Storage'
		};

		//  Hypertext Transfer Protocol - HTTP/1.1
		//  http://tools.ietf.org/html/2616
		static $rfc2616 = ImmMap<int, string> {
			/**  3xx  -  Redirection  **/

			//  The 306 status code was used in a previous version of the specification, is no longer used, and the
			//  code is reserved.
			306 => '(Unused)',

			//  The requested resource resides temporarily under a different URI. Since the redirection MAY be
			//  altered on occasion, the client SHOULD continue to use the Request-URI for future requests. This
			//  response is only cacheable if indicated by a Cache-Control or Expires header field. The temporary
			//  URI SHOULD be given by the Location field in the response. Unless the request method was HEAD, the
			//  entity of the response SHOULD contain a short hypertext note with a hyperlink to the new URI(s) ,
			//  since many pre-HTTP/1.1 user agents do not understand the 307 status. Therefore, the note SHOULD
			//  contain the information necessary for a user to repeat the original request on the new URI. If the
			//  307 status code is received in response to a request other than GET or HEAD, the user agent MUST NOT
			//  automatically redirect the request unless it can be confirmed by the user, since this might change
			//  the conditions under which the request was issued.
			307 => 'Temporary Redirect',

			/**  4xx  -  User Error  **/

			//  A server SHOULD return a response with this status code if a request included a Range request-header
			//  field (section 14.35), and none of the range-specifier values in this field overlap the current
			//  extent of the selected resource, and the request did not include an If-Range request-header field.
			//  (For byte-ranges, this means that the first-byte-pos of all of the byte-range-spec values were
			//  greater than the current length of the selected resource.) When this status code is returned for a
			//  byte-range request, the response SHOULD include a Content-Range entity-header field specifying the
			//  current length of the selected resource. This response MUST NOT use the
			//  multipart/byteranges content-type.
			416 => 'Requested Range Not Satisfiable',

			//  The expectation given in an Expect request-header field could not be met by this server, or, if the
			//  server is a proxy, the server has unambiguous evidence that the request could not be met by the
			//  next-hop server.
			417 => 'Expectation Failed'
		};

		//  An HTTP Extension Framework
		//  http://tools.ietf.org/html/2774
		static $rfc2774 = ImmMap<int, string> {
			/**  5xx  -  Server Error  **/

			//  The policy for accessing the resource has not been met in the request. The server should send back
			//  all the information necessary for the client to issue an extended request. It is outside the scope
			//  of this specification to specify how the extensions inform the client. If the 510 response contains
			//  information about extensions that were not present in the initial request then the client MAY repeat
			//  the request if it has reason to believe it can fulfill the extension policy by modifying the request
			//  according to the information provided in the 510 response. Otherwise the client MAY present any
			//  entity included in the 510 response to the user, since that entity may include relevant
			//  diagnostic information.
			510 => 'Not Extended'
		};

		//  Upgrading to TLS Within HTTP/1.1
		//  http://tools.ietf.org/html/2817
		static $rfc2817 = ImmMap<int, string> {
			/**  4xx  -  User Error  **/

			//  Reliable, interoperable negotiation of Upgrade features requires an unambiguous failure signal. The
			//  426 Upgrade Required status code allows a server to definitively state the precise protocol
			//  extensions a given resource must be served with.
			426 => 'Upgrade Required'
		};

		//  HTTP Over TLS
		//  http://tools.ietf.org/html/2818
		static $rfc2818 = ImmMap<int, string> {
			//  RFC 2818 does not add new status codes
		};

		//  Delta encoding in HTTP (HTTP/1.1 extension)
		//  http://tools.ietf.org/html/3229
		static $rfc3229 = ImmMap<int, string> {
			/**  2xx  -  Successful  **/

			//  The server has fulfilled a GET request for the resource, and the response is a representation of the
			//  result of one or more instance-manipulations applied to the current instance. The actual current
			//  instance might not be available except by combining this response with other previous or future
			//  responses, as appropriate for the specific instance-manipulation(s).
			226 => 'IM Used'
		};

		//  Calendaring Extensions to WebDAV (CalDAV)
		//  http://tools.ietf.org/html/4791
		static $rfc4791 = ImmMap<int, string> {
			//  RFC 4791 does not add new status codes
		};

		//  HTTP Extensions for Web Distributed Authoring and Versioning (WebDAV)
		//  http://tools.ietf.org/html/4918
		static $rfc4918 = ImmMap<int, string> {
			//  RFC 4918 does not add new status codes
		};

		//  Extended MKCOL for Web Distributed Authoring and Versioning (WebDAV)
		//  http://tools.ietf.org/html/5689
		static $rfc5689 = ImmMap<int, string> {
			//  RFC 5689 does not add new status codes
		};

		//  Defining Well-Known Uniform Resource Identifiers (URIs)
		//  http://tools.ietf.org/html/5785
		static $rfc5785 = ImmMap<int, string> {
			//  RFC 5785 does not add new status codes
		};

		//  Binding Extensions to Web Distributed Authoring and Versioning (WebDAV)
		//  http://tools.ietf.org/html/5842
		static $rfc5842 = ImmMap<int, string> {
			/**  2xx  -  Successful  **/

			//  The 208 (Already Reported) status code can be used inside a DAV: propstat response element to avoid
			//  enumerating the internal members of multiple bindings to the same collection repeatedly. For each
			//  binding to a collection inside the request's scope, only one will be reported with a 200 status,
			//  while subsequent DAV:response elements for all other bindings will use the 208 status, and no
			//  DAV:response elements for their descendants are included.
			208 => 'Already Reported',

			/**  5xx  -  Server Error  **/

			//  The 508 status code indicates that the server terminated an operation because it encountered an
			//  infinite loop while processing a request with 'Depth: infinity'. This status indicates that the
			//  entire operation failed.
			508 => 'Loop Detected'
		};

		//  Use of the Content-Disposition Header Field in the Hypertext Transfer Protocol (HTTP)
		//  http://tools.ietf.org/html/6266
		static $rfc6266 = ImmMap<int, string> {
			//  RFC 6266 does not add new status codes
		};

		//  Additional HTTP Status Codes
		//  http://tools.ietf.org/html/6585
		static $rfc6585 = ImmMap<int, string> {
			/**  4xx  -  User Error  **/

			//  The 428 status code indicates that the origin server requires the request to be conditional. Its
			//  typical use is to avoid the 'lost update' problem, where a client GETs a resource's state, modifies
			//  it, and PUTs it back to the server, when meanwhile a third party has modified the state on the
			//  server, leading to a conflict. By requiring requests to be conditional, the server can assure that
			//  clients are working with the correct copies. Responses using this status code SHOULD explain how to
			//  resubmit the request successfully. Responses with the 428 status code MUST NOT be stored by
			//  a cache.
			428 => 'Precondition Required',

			//  The 429 status code indicates that the user has sent too many requests in a given amount of time
			//  ('rate limiting'). The response representations SHOULD include details explaining the condition, and
			//  MAY include a Retry-After header indicating how long to wait before making a new request. Responses
			//  with the 429 status code MUST NOT be stored by a cache.
			429 => 'Too Many Requests',

			//  The 431 status code indicates that the server is unwilling to process the request because its header
			//  fields are too large. The request MAY be resubmitted after reducing the size of the request header
			//  fields. It can be used both when the set of request header fields in total is too large, and when a
			//  single header field is at fault. In the latter case, the response representation SHOULD specify
			//  which header field was too large. Responses with the 431 status code MUST NOT be stored by
			//  a cache.
			431 => 'Request Header Fields Too Large',

			/**  5xx  -  Server Error  **/

			//  The 511 status code indicates that the client needs to authenticate to gain network access. The
			//  response representation SHOULD contain a link to a resource that allows the user to submit
			//  credentials (e.g., with an HTML form). Note that the 511 response SHOULD NOT contain a challenge or
			//  the login interface itself, because browsers would show the login interface as being associated with
			//  the originally requested URL, which may cause confusion. The 511 status SHOULD NOT be generated by
			//  origin servers; it is intended for use by intercepting proxies that are interposed as a means of
			//  controlling access to the network. Responses with the 511 status code MUST NOT be stored by
			//  a cache.
			511 => 'Network Authentication Required'
		};

		//  Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing
		//  http://tools.ietf.org/html/7230
		static $rfc7230 = ImmMap<int, string> {
			//  RFC 7230 does not add new status codes
		};

		//  Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content
		//  http://tools.ietf.org/html/7231
		static $rfc7231 = ImmMap<int, string> {
			/**  3xx  -  Redirection  **/

			//  The 302 (Found) status code indicates that the target resource resides temporarily under a different
			//  URI. Since the redirection might be altered on occasion, the client ought to continue to use the
			//  effective request URI for future requests. The server SHOULD generate a Location header field in the
			//  response containing a URI reference for the different URI. The user agent MAY use the Location field
			//  value for automatic redirection. The server's response payload usually contains a short hypertext
			//  note with a hyperlink to the different URI(s). Note: For historical reasons, a user agent MAY change
			//  the request method from POST to GET for the subsequent request. If this behavior is undesired, the
			//  307 (Temporary Redirect) status code can be used instead.
			302 => 'Found',

			/**  4xx  -  User Error  **/

			//  The 413 (Payload Too Large) status code indicates that the server is refusing to process a request
			//  because the request payload is larger than the server is willing or able to process. The server MAY
			//  close the connection to prevent the client from continuing the request. If the condition is
			//  temporary, the server SHOULD generate a Retry-After header field to indicate that it is temporary
			//  and after what time the client MAY try again.
			413 => 'Payload Too Large',

			//  The 414 (URI Too Long) status code indicates that the server is refusing to service the request
			//  because the request-target is longer than the server is willing to interpret. This rare condition is
			//  only likely to occur when a client has improperly converted a POST request to a GET request with
			//  long query information, when the client has descended into a 'black hole' of redirection (e.g., a
			//  redirected URI prefix that points to a suffix of itself) or when the server is under attack by a
			//  client attempting to exploit potential security holes.
			414 => 'URI Too Long'
		};

		//  Hypertext Transfer Protocol (HTTP/1.1): Conditional Requests
		//  http://tools.ietf.org/html/7232
		static $rfc7232 = ImmMap<int, string> {
			//  RFC 7232 does not add new status codes
		};

		//  Hypertext Transfer Protocol (HTTP/1.1): Range Requests
		//  http://tools.ietf.org/html/7233
		static $rfc7233 = ImmMap<int, string> {
			//  RFC 7233 does not add new status codes
		};

		//  Hypertext Transfer Protocol (HTTP/1.1): Caching
		//  http://tools.ietf.org/html/7234
		static $rfc7234 = ImmMap<int, string> {
			//  RFC 7234 does not add new status codes
		};

		//  Hypertext Transfer Protocol (HTTP/1.1): Authentication
		//  http://tools.ietf.org/html/7235
		static $rfc7235 = ImmMap<int, string> {
			//  RFC 7235 does not add new status codes
		};

		//  The Hypertext Transfer Protocol Status Code 308 (Permanent Redirect)
		//  http://tools.ietf.org/html/7238
		static $rfc7238 = ImmMap<int, string> {
			//  RFC 7238 does not add new status codes
		};

		//  The Hypertext Transfer Protocol Status Code 308 (Permanent Redirect)
		//  http://tools.ietf.org/html/7538
		static $rfc7538 = ImmMap<int, string> {
			/**  3xx  -  Redirection  **/

			//  The 308 (Permanent Redirect) status code indicates that the target resource has been assigned a new
			//  permanent URI and any future references to this resource ought to use one of the enclosed URIs.
			//  Clients with link editing capabilities ought to automatically re-link references to the effective
			//  request URI to one or more of the new references sent by the server, where possible. The server
			//  SHOULD generate a Location header field in the response containing a preferred URI reference for the
			//  new permanent URI. The user agent MAY use the Location field value for automatic redirection. The
			//  server's response payload usually contains a short hypertext note with a hyperlink to the new
			//  URI(s). A 308 response is cacheable by default; i.e., unless otherwise indicated by the method
			//  definition or explicit cache controls. Note: This status code is similar to 301 (Moved Permanently)
			//  except that it does not allow changing the request method from POST to GET.
			308 => 'Permanent Redirect'
		};

		//  Hypertext Transfer Protocol Version 2 (HTTP/2)
		//  http://tools.ietf.org/html/7540
		static $rfc7540 = ImmMap<int, string> {
			/**  4xx  -  User Error  **/

			//  The 421 (Misdirected Request) status code indicates that the request was directed at a server that
			//  is not able to produce a response. This can be sent by a server that is not configured to produce
			//  responses for the combination of scheme and authority that are included in the request URI. Clients
			//  receiving a 421 (Misdirected Request) response from a server MAY retry the request - whether the
			//  request method is idempotent or not - over a different connection. This is possible if a connection
			//  is reused (Section 9.1.1) or if an alternative service is selected This status code MUST NOT be
			//  generated by proxies. A 421 response is cacheable by default, i.e., unless otherwise indicated by
			//  the method definition or explicit cache controls
			421 => 'Misdirected Request'
		};

		//  Miscellanious Status Codes
		//  https://tools.ietf.org/html/draft-tbray-http-legally-restricted-status-00
		static $misc = ImmMap<int, string> {
			/**  4xx  -  User Error  **/

			//  This status code indicates that the server is subject to legal restrictions which prevent it
			//  servicing the request. Since such restrictions typically apply to all operators in a legal
			//  jurisdiction, the server in question may or may not be an origin server. The restrictions typically
			//  most directly affect the operations of ISPs and search engines. Responses using this status code
			//  SHOULD include an explanation, in the response body, of the details of the legal restriction; which
			//  legal authority is imposing it, and what class of resources it applies to.
			451 => 'Unavailable For Legal Reasons'
		};

		switch (strToUpper($protocol)) {
			case '1.0':
			case 'HTTP/1.0':
				$result->addAll($rfc1945->items());  //  Hypertext Transfer Protocol - HTTP/1.0
				break;

			//  The entire HTTP/1.1 spec was revised in later RFCs (7230-7235), however did this not explicitly
			//  update the WebDAV spec, and it seems right to have the HTTP/1.1 spec up to 2014 at hand
			case '1.1A':
			case 'HTTP/1.1A':
				//  inherit all from http/1.0
				$result = $this->_getProtocolDefinitions('http/1.0')->toMap();
				$result->addAll($rfc2068->items());  //  Hypertext Transfer Protocol - HTTP/1.1
				$result->addAll($rfc2616->items());  //  Hypertext Transfer Protocol - HTTP/1.1
				$result->addAll($rfc2817->items());  //  Upgrading to TLS Within HTTP/1.1
				$result->addAll($rfc5785->items());  //  Defining Well-Known Uniform Resource Identifiers (URIs)
				$result->addAll($rfc6266->items());  //  Use of the Content-Disposition Header Field in the Hypertext Transfer Protocol (HTTP)
				$result->addAll($rfc6585->items());  //  Additional HTTP Status Codes
				break;

			case '1.1':
			case 'HTTP/1.1':
				//  inherit all from http/1.1a
				$result = $this->_getProtocolDefinitions('http/1.1a')->toMap();
				$result->addAll($rfc7230->items());  //  Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing
				$result->addAll($rfc7231->items());  //  Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content
				$result->addAll($rfc7232->items());  //  Hypertext Transfer Protocol (HTTP/1.1): Conditional Requests
				$result->addAll($rfc7233->items());  //  Hypertext Transfer Protocol (HTTP/1.1): Range Requests
				$result->addAll($rfc7234->items());  //  Hypertext Transfer Protocol (HTTP/1.1): Caching
				$result->addAll($rfc7235->items());  //  Hypertext Transfer Protocol (HTTP/1.1): Authentication
				$result->addAll($rfc7538->items());  //  The Hypertext Transfer Protocol Status Code 308 (Permanent Redirect)
				break;

			case 'H2':
			case 'H2C':
			case '2':
			case '2.0':
			case 'HTTP/2.0':
			case 'HTTP/2':
				//  inherit all from http/1.1
				$result = $this->_getProtocolDefinitions('http/1.1')->toMap();
				$result->addAll($rfc7540->items());  //  Hypertext Transfer Protocol Version 2 (HTTP/2)
				break;

			//  The WebDAV spec does not appear to be updated to the new HTTP/1.1 specifications, hence we extend
			//  our HTTP/1.1a version, which reflects the state of HTTP/1.1 up to 2014. The main concern is the
			//  absence of the 308 status, defined in RFC 7538
			case 'DAV':
			case 'WEBDAV':
				//  inherit all from http/1.1a
				$result = $this->_getProtocolDefinitions('http/1.1a')->toMap();
				$result->addAll($rfc2518->items());  //  HTTP Extensions for Distributed Authoring - WEBDAV
				$result->addAll($rfc3229->items());  //  Delta encoding in HTTP (HTTP/1.1 extension)
				$result->addAll($rfc4791->items());  //  Calendaring Extensions to WebDAV (CalDAV)
				$result->addAll($rfc4918->items());  //  HTTP Extensions for Web Distributed Authoring and Versioning (WebDAV)
				$result->addAll($rfc5689->items());  //  Extended MKCOL for Web Distributed Authoring and Versioning (WebDAV)
				$result->addAll($rfc5842->items());  //  Binding Extensions to Web Distributed Authoring and Versioning (WebDAV)
				break;

			case 'HTCPCP/1.0':
				//  inherit all from http/1.0
				$result = $this->_getProtocolDefinitions('http/1.0')->toMap();
				$result->addAll($rfc2324->items());  //  Hyper Text Coffee Pot Control Protocol (HTCPCP/1.0)
				break;

			case 'ALL':
				//  create a new result Map
				$result = Map<int, string> {};
				$result->addAll($rfc1945->items());  //  Hypertext Transfer Protocol - HTTP/1.0
				$result->addAll($rfc2068->items());  //  Hypertext Transfer Protocol - HTTP/1.1
				$result->addAll($rfc2295->items());  //  Transparent Content Negotiation in HTTP
				$result->addAll($rfc2324->items());  //  Hyper Text Coffee Pot Control Protocol (HTCPCP/1.0)
				$result->addAll($rfc2518->items());  //  HTTP Extensions for Distributed Authoring - WEBDAV
				$result->addAll($rfc2616->items());  //  Hypertext Transfer Protocol - HTTP/1.1
				$result->addAll($rfc2774->items());  //  An HTTP Extension Framework
				$result->addAll($rfc2817->items());  //  Upgrading to TLS Within HTTP/1.1
				$result->addAll($rfc2818->items());  //  HTTP Over TLS
				$result->addAll($rfc3229->items());  //  Delta encoding in HTTP (HTTP/1.1 extension)
				$result->addAll($rfc4791->items());  //  Calendaring Extensions to WebDAV (CalDAV)
				$result->addAll($rfc4918->items());  //  HTTP Extensions for Web Distributed Authoring and Versioning (WebDAV)
				$result->addAll($rfc5689->items());  //  Extended MKCOL for Web Distributed Authoring and Versioning (WebDAV)
				$result->addAll($rfc5785->items());  //  Defining Well-Known Uniform Resource Identifiers (URIs)
				$result->addAll($rfc5842->items());  //  Binding Extensions to Web Distributed Authoring and Versioning (WebDAV)
				$result->addAll($rfc6266->items());  //  Use of the Content-Disposition Header Field in the Hypertext Transfer Protocol (HTTP)
				$result->addAll($rfc6585->items());  //  Additional HTTP Status Codes
				$result->addAll($rfc7230->items());  //  Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing
				$result->addAll($rfc7231->items());  //  Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content
				$result->addAll($rfc7232->items());  //  Hypertext Transfer Protocol (HTTP/1.1): Conditional Requests
				$result->addAll($rfc7233->items());  //  Hypertext Transfer Protocol (HTTP/1.1): Range Requests
				$result->addAll($rfc7234->items());  //  Hypertext Transfer Protocol (HTTP/1.1): Caching
				$result->addAll($rfc7235->items());  //  Hypertext Transfer Protocol (HTTP/1.1): Authentication
				$result->addAll($rfc7238->items());  //  The Hypertext Transfer Protocol Status Code 308 (Permanent Redirect)
				$result->addAll($rfc7538->items());  //  The Hypertext Transfer Protocol Status Code 308 (Permanent Redirect)
				$result->addAll($rfc7540->items());  //  Hypertext Transfer Protocol Version 2 (HTTP/2)
				$result->addAll($misc->items());     //  Miscellanious Status Codes
				break;

			default:
				//  create a new result Map
				$result = Map<int, string> {};
				break;
		}

		if ($sort) {
			ksort($result);
		}

		return $result->toImmMap();
	}
}
