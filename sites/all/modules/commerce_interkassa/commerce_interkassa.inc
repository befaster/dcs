<?php

/**
 * $icon = '<img src="/'.drupal_get_path('module', 'commerce_victoria').'/images/logo.png">';|
 * @file
 *
 * @author greenSkin
 */

/*
 * 
https://psvictoria.com/merchant/invoice/acc/U554729035058/amount/100/currency/UAH/storeName/merchant/successUrl/commerce.l%252Fcommerce_victoria%252Fsuccess/successMethod/POST/failUrl/commerce.l%252Fcommerce_victoria%252Ffail/failMethod/POST/statusUrl/commerce.l%252Fcommerce_victoria%252Fstatus/statusMethod/POST

<a href='https://psvictoria.com/merchant/invoice/acc/U554729035058/amount/100/currency/UAH/storeName/merchant/successUrl/commerce.l%252Fcommerce_victoria%252Fsuccess/successMethod/POST/failUrl/commerce.l%252Fcommerce_victoria%252Ffail/failMethod/POST/statusUrl/commerce.l%252Fcommerce_victoria%252Fstatus/statusMethod/POST'>Pay with Victoria</a> 
	
<form action='https://psvictoria.com/merchant/invoice' method='POST'>
<input type='hidden' name='acc' value='U554729035058' />
<input type='hidden' name='amount' value='100' />
<input type='hidden' name='currency' value='UAH' />
<input type='hidden' name='storeName' value='merchant' />
<input type='hidden' name='successUrl' value='commerce.l/commerce_victoria/success' />
<input type='hidden' name='successMethod' value='POST' />
<input type='hidden' name='failUrl' value='commerce.l/commerce_victoria/fail' />
<input type='hidden' name='failMethod' value='POST' />
<input type='hidden' name='statusUrl' value='commerce.l/commerce_victoria/status' />
<input type='hidden' name='statusMethod' value='POST' />
<input type='submit' />
</form>
 

//Balance:XML

Input
POST address: https://psvictoria.com/api/balance/format/xml
Package: param name = package
apiUid: UID of API
apiSecret: MD5(API secret code) of API
purseBalance: Purse No

Output
result: SUCCESS | FAIL
purseBalance: balance
errorCode: Code of Error
errorMsg: Message of Error

Example
	<form action='https://psvictoria.com/api/balance/format/xml' method='post'>
	<textarea name='package'>
	<?xml version="1.0"?>
	<apiPackage id='123456'>
	 <apiUid>Il9ixo2yDaOhAbiMs86a57KUQBjabnvRftXpgbtm580aY9hTFM</apiUid>
	 <apiSecret>fcea920f7412b5da7be0cf42b8c93759</apiSecret>
	 <purseBalance>S012745918633</purseBalance>
	 </apiPackage>
	</textarea>
	<input type='submit' />
	</form>
	
	<?xml version="1.0"?>
	<apiPackage id="123456"><result>SUCCESS</result><purseBalance>2369.97</purseBalance></apiPackage>
	
	<?xml version="1.0"?>
	<apiPackage id="123456"><result>FAIL</result><errorCode>301</errorCode><errorMsg></errorMsg></apiPackage>
	
Transfer:XML

Input
POST address: https://psvictoria.com/api/transfer/format/xml
Package: param name = package
apiUid: UID of API
apiSecret: MD5(API secret code) of API
pursePayer: Payer Purse No
pursePayee: Payee Purse No
amount: Amount
details: Details

Output
result: SUCCESS | FAIL
tranId: Id of transaction
errorCode: Code of Error
errorMsg: Message of Error

Example
	<form action='https://psvictoria.com/api/transfer/format/xml' method='post'>
	<textarea name='package'>
	<?xml version="1.0"?>
	<apiPackage id='123456'>
	 <apiUid>Il9ixo2yDaOhAbiMs86a57KUQBjabnvRftXpgbtm580aY9hTFM</apiUid>
	 <apiSecret>fcea920f7412b5da7be0cf42b8c93759</apiSecret>
	 <transfer>
	 <pursePayer>S012745918633</pursePayer>
	 <pursePayee>S018967921073</pursePayee>
	 <amount>100.23</amount>
	 <details>+100; #475347537</details>
	 </transfer>
	 </apiPackage>
	</textarea>
	<input type='submit' />
	</form>
	
	<?xml version="1.0"?>
	<apiPackage id="123456"><result>SUCCESS</result><tranId>gA4RRDd2Zs9dfmMlDQdSXq4DfAPYL5xx</tranId></apiPackage>
	
	<?xml version="1.0"?>
	<apiPackage id="123456"><result>FAIL</result><errorCode>200</errorCode><errorMsg>Payee not found</errorMsg></apiPackage>
	
Error Codes

100: Authentication invalid
101: API disabled
102: IP forbidden
103: API method forbidden
200: Transfer failed
300: Invalid input parameters
301: Invalid Purse No

 */