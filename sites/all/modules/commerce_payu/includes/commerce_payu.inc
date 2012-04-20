<?php
/**
 * @file
 * Implements PayU Payments in Drupal Commerce checkout.
 */

function commerce_payu_close_transaction() {
  /*
   stała opis
   %transId% identyfikator nowej transakcji utworzonej w aplikacji PayU
   %posId% wartosci pos id
   %payType% wartosci pay type
   %sessionId% wartosci session id
   %amountPS% wartosci amount - jako separator kropka
   %amountCS% wartosci amount - jako separator przecinek
   %orderId% wartosci order id
   %error% numer błedu zgodnie z tabelka 2.1 (s. 3), jest wykorzystywany tylko przy

   */
  $error_id = isset($_REQUEST['error']) ? $_REQUEST['error'] : 0;
  $order_id = isset($_REQUEST['order_id']) ? $_REQUEST['order_id'] : 0;
  $session_id = isset($_REQUEST['session_id']) ? $_REQUEST['session_id'] : 0;

  $a_session_id = explode(':', $session_id, 2);
  $payment_redirect_key = (count($a_session_id)==2) ? $a_session_id[1]: '';

  $url_error = "checkout/$order_id/payment/back/$payment_redirect_key";
  $url_ok = "checkout/$order_id/payment/return/$payment_redirect_key";

  if ($error_id) {
    drupal_set_message(t('Transaction unsuccessful') . ': ' . _commerce_payu_get_error_description($error_id), 'error');
    drupal_goto($url_error);

  }
  else {
    drupal_goto($url_ok);
  }
}

/**
 * menu callback for raports platnosc.pl
 *
 */
function commerce_payu_online() {
  $pos_id = isset($_POST['pos_id']) ? check_plain($_POST['pos_id']): 0;
  $session_id = isset($_POST['session_id']) ? check_plain($_POST['session_id']): 0;
  $ts = isset($_POST['ts']) ? check_plain($_POST['ts']): 0;
  $sig = isset($_POST['sig']) ? check_plain($_POST['sig']): 0;

  // debug
  //print '<pre>'.print_r($_POST, 1).'<pre>';

  $watchdog_variables = array('@session_id' => $session_id);
  $watchdog_urls = '';

  if (!$session_id) {
    watchdog('commerce_payu', 'ERROR empty session_id', $watchdog_variables, WATCHDOG_WARNING);
    print t('ERROR empty session_id');
    exit();
  }

  $a_session_id = explode(':', $session_id, 2);
  $order_id = intval($a_session_id[0]);
  $payment_redirect_key = (count($a_session_id)==2) ? $a_session_id[1]: '';

  if (!$order_id) {
    watchdog('commerce_payu', 'ERROR order_id', $watchdog_variables, WATCHDOG_WARNING);
    print t('ERROR order_id');
    exit();
  }

  $order = commerce_order_load($order_id);
  $payment_method_instance_id = $order->data['payment_method'];
  $payment_method = commerce_payment_method_instance_load($payment_method_instance_id);
  // debug
  //print "payment_method_instance_id: $payment_method_instance_id<br/>";
  //print '<strong>payment_method</strong><pre>'.print_r($payment_method, 1).'</pre>';
  //print '<strong>order</strong><pre>'.print_r($order, 1).'</pre>';


  if ($pos_id != $payment_method['settings']['pos_id']) {
    // bad pos_id (shop)
    $watchdog_variables['@pos_id'] = $pos_id;
    watchdog('commerce_payu', 'ERROR bad pos_id: @pos_id.', $watchdog_variables, WATCHDOG_WARNING);
    print t('ERROR BAD pos_id: @pos_id.', $watchdog_variables);
    exit();
  }

  if (md5( $pos_id . $session_id . $ts . $payment_method['settings']['md5_2']) != $sig) {
    $to_sig = $pos_id . $session_id . $ts . $payment_method['settings']['md5_2'];
    $sig_local = md5( $pos_id . $session_id . $ts . $payment_method['settings']['md5_2']);

    $watchdog_variables['@pos_id'] = $pos_id;
    $watchdog_variables['@ts'] = $ts;
    $watchdog_variables['@commerce_payu_md5_2'] = $payment_method['settings']['md5_2'];
    $watchdog_variables['@sig'] = $sig;
    $watchdog_variables['@sig_local'] = $sig_local;
    $watchdog_variables['@to_sig'] = $to_sig;
    watchdog('commerce_payu',
      'PayU transaction @session_id sig error. to_sig=[@to_sig] (@pos_id, @session_id, @ts, commerce_payu_md5_2, @sig, @sig_local)'
      , $watchdog_variables, WATCHDOG_WARNING, $watchdog_urls);
    print t('ERROR: sig, see to log');
    exit();
  }

  $url = "https://www.platnosci.pl/paygw/UTF/Payment/get";

  $ts = time();
  $sig = md5( $pos_id . $session_id . $ts . $payment_method['settings']['md5']);
  $data = "&pos_id=" . $pos_id . "&session_id=" . urlencode($session_id) . "&ts=" . $ts . "&sig=" . $sig;

  $assoc_options = array(
    'headers' => array('Content-Type' => 'application/x-www-form-urlencoded; charset=utf-8'),
    'method' => 'POST',
    'data' => $data,
  );
  $result = drupal_http_request($url, $assoc_options);

  if ($result->code != 200) {
    $watchdog_variables['@serialize'] = serialize($result);
    watchdog('commerce_payu', 'Error sending confirmation to PayU: @session_id @serialize', $watchdog_variables, WATCHDOG_WARNING, $watchdog_urls);
    print t('ERROR: @code, see to drupal log', array('@code' => $result->code));
    exit;
  }

  $remote = _commerce_payu_parse_xml_raport($result->data);
  //print '<strong>remote:</strong><pre>'.print_r($remote, 1).'</pre>';
  //exit;
  if ($remote['status'] == 'OK') {
    $watchdog_urls = implode(', ', array(l(t('view in PayU'),
      _commerce_payu_details_url($remote['trans']['id']))));


    if ($remote['trans']['pos_id'] != $payment_method['settings']['pos_id']) {
      $watchdog_variables['@pos_id'] = $pos_id;
      $watchdog_variables['@remote_pos_id'] = $remote['trans']['pos_id'];
      watchdog('commerce_payu', 'Error błędny numer POS_ID: @pos_id<>@remote_pos_id', $watchdog_variables, WATCHDOG_WARNING, $watchdog_urls);
      print t('ERROR: bad @POS_ID', array('@pos_id' => $remote['trans']['pos_id']));
      exit;
    }
    $sig = md5($remote['trans']['pos_id'] . $remote['trans']['session_id'] .
      $remote['trans']['order_id'] . $remote['trans']['status'] .
      $remote['trans']['amount'] . htmlspecialchars_decode($remote['trans']['desc']) .
      $remote['trans']['ts'] . $payment_method['settings']['md5_2']);

    if ($remote['trans']['sig'] != $sig) {
      watchdog('commerce_payu', 'Error bledny podpis statusu transakcji', $watchdog_variables, WATCHDOG_WARNING, $watchdog_urls);
      print t('ERROR: błędny podpis');
      exit;
    }

    if ($remote['trans']['status']) {  //--- payment status recognized
      //$remote_id = $remote['trans']['id'];
      //$pos_id = $remote['trans']['pos_id'];
      $desc = $remote['trans']['desc'];
      $ts = $remote['trans']['ts'];
      $sig = $remote['trans']['sig'];

      $status_description = _commerce_payu_get_status_description($remote['trans']['status']);

      $watchdog_variables['@status_id'] = $remote['trans']['status'];
      $watchdog_variables['@status_description'] = $status_description;
      watchdog('commerce_payu', 'transaction: @session_id, status_description: @status_description, status_id: @status_id', $watchdog_variables, WATCHDOG_INFO, $watchdog_urls);

      foreach (commerce_payment_transaction_load_multiple(
        array(),
        array('remote_id' => $remote['trans']['id'])) as $transaction) {
      }

      if (empty($transaction)) {
        $transaction = commerce_payment_transaction_new('payu', $order->order_id);
        $transaction->created = time();
        //print 'empty</br>';
      }
      $transaction->instance_id = $payment_method['instance_id'];

      $transaction->remote_id = $remote['trans']['id'];
      $transaction->amount = $remote['trans']['amount']*10;//commerce_currency_decimal_to_amount($ipn['mc_gross'], $ipn['mc_currency']);
      $transaction->currency_code = 'PL';
      $transaction->payload[REQUEST_TIME] = print_r($_POST, 1) . "\n\n" . $result->data;

      $transaction->changed = time();

      $transaction->message = t('The payment status @status_description.' . $watchdog_urls, $watchdog_variables);
      $transaction->remote_status = $status_description;
      //print '<strong>transaction</strong><pre>'.print_r($transaction, 1).'</pre>';

      switch ($remote['trans']['status']) {
        case '1': // new
          $transaction->status = COMMERCE_PAYMENT_STATUS_PENDING;
          break;
        case '4': // Started
          $transaction->status = COMMERCE_PAYMENT_STATUS_PENDING;
          break;
        case '5': // Awaiting reception
          $transaction->status = COMMERCE_PAYMENT_STATUS_PENDING;
          break;
        case '2': // Cancelled
        case '3': // Rejected
        case '6': // Negative authorization
        case '7': // Payment rejected
          $transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
          break;
        case '99': // Payment received
          $transaction->status = COMMERCE_PAYMENT_STATUS_SUCCESS;
          // user is not returned to the store, but paid the means, we close the transaction
          if($order->status == 'checkout_payment') {
            commerce_checkout_complete($order);
          }
          break;
        case '888': // Incorrect status
          $transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
          break;
        default:
          $transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
          break;
      }
      //print '<strong>transaction</strong><pre>'.print_r($transaction, 1).'</pre>';
      commerce_payment_transaction_save($transaction);
      echo "OK";
      exit;
    }
    else {
      watchdog('commerce_payu', 'unknown status', $watchdog_variables, WATCHDOG_WARNING, $watchdog_urls);
      print t('ERROR: unknown status');
      exit;
    }
  }
  else {
    // notification will be resend by PayU
    if (!empty($remote['error']['nr'])) {
      $watchdog_variables['@code'] = $session_id;
      $watchdog_variables['@desc'] = _commerce_payu_get_error_description($remote['error']['nr']);
      $watchdog_variables['@message'] = $remote['error']['message'];
      watchdog('commerce_payu', 'Error sending confirmation to PayU: @session_id @code, @desc, @message', $watchdog_variables, WATCHDOG_WARNING, $watchdog_urls);
      print t('ERROR: @code, @desc, @message. See to drupal log', $watchdog_variables);
      exit;
    }
    else {
      $watchdog_variables['@serialize'] = serialize($result);
      watchdog('commerce_payu', 'Error sending confirmation to PayU: @session_id @serialize', $watchdog_variables, WATCHDOG_WARNING, $watchdog_urls);
      print t('ERROR: Unknown, see Watchdog1');
      exit;
    }
  }
  watchdog('commerce_payu', 'ERROR: Unknown, see Watchdog', $watchdog_variables, WATCHDOG_WARNING, $watchdog_urls);
  print t('ERROR: Unknown, see Watchdog2');
  exit;
}


function _commerce_payu_parse_xml_raport($xml_raport) {
  $remote = array();

  $xml_obj = simplexml_load_string($xml_raport);
  $data = get_object_vars($xml_obj);
  $remote['status'] = isset($data['status']) ? $data['status'] : 'UNKOWN';
  $trans = get_object_vars($xml_obj->trans);
  foreach ($trans as $key => $val) {
    if (is_object($val)) {
      $remote['trans'][$key] = '';
    }
    else {
      $remote['trans'][$key] = $val;
    }
  }
  $trans = get_object_vars($xml_obj->error);
  foreach ($trans as $key => $val) {
    if (is_object($val)) {
      $remote['error'][$key] = '';
    }
    else {
      $remote['error'][$key] = $val;
    }
  }
  return $remote;
}


// ----

/**
 * List Type Status
 *
 * @return array
 */
function _commerce_payu_status_types() {
  return array(
    '0' => t('Not started'),
    '1' => t('New'),
    '2' => t('Cancelled'),
    '3' => t('Rejected'),
    '4' => t('Started'),
    '5' => t('Awaiting collection'),
    '6' => t('Negative authorization'),
    '7' => t('Payment rejected'),
    '99' => t('Payment collected'),
    '888' => t('Incorrect status'),
  );
}

/**
 * Error description list
 *
 * @return array
 */
function _commerce_payu_error_description() {
  return array(
    '100' => t('Missing or incorrect value of pos id parameter'),
    '101' => t('Missing session_id parameter'),
    '102' => t('Missing ts parameter'),
    '103' => t('Missing or incorrect value of sig parameter'),
    '104' => t('Missing desc parameter'),
    '105' => t('Missing client_ip parameter'),
    '106' => t('Missing first_name parameter'),
    '107' => t('Missing last_name parameter'),
    '108' => t('Missing street parameter'),
    '109' => t('Missing city parameter'),
    '110' => t('No post code variable.'),
    '111' => t('Missing amount parameter'),
    '112' => t('Incorrect bank account numberr'),
    '113' => t('Missing e-mail parameter'),
    '114' => t('Missing phone number'),
    '200' => t('Other temporary error'),
    '201' => t('Other temporary database error'),
    '202' => t('Pos with the specified id is blocked'),
    '203' => t('Forbidden pay type value for given pos id'),
    '204' => t('Specified payment method (pay_type value) is temporarily blocked for the given pos_id, e.g. 204 maintenance break of payment gateway'),
    '205' => t('Transaction amount lower than the minimal value'),
    '206' => t('Transaction amount higher than the maximal value'),
    '207' => t('Exceeded value of all transactions for one Client in the last period of time'),
    '208' => t('POS operates in ExpressPayment variant but this form of cooperation has not been activated
(awaiting consent of customer support department)'),
    '209' => t('Incorrect pos_id or pos_auth_key number'),
    '500' => t('No such transaction'),
    '501' => t('Transaction started earlier'),
    '502' => t('Transaction has already been authorised'),
    '503' => t('Transaction cancelled earlier'),
    '504' => t('Transaction previously forwarded for collection'),
    '505' => t('Transaction already collected'),
    '506' => t('Transaction started earlier'),
    '507' => t('Error while returning funds to Client'),
    '599' => t('Incorrect transaction status, e.g. transaction cannot be acknowledged several times or other, please contact us'),
    '999' => t('Other critical error – please contact site administrator'),
  );
}


function _commerce_payu_get_status_description($status_id) {
  $status_types = _commerce_payu_status_types();
  return isset($status_types[$status_id]) ? $status_types[$status_id] : t('unknown');
}

function _commerce_payu_get_error_description($error_id) {
  $error_description = _commerce_payu_error_description();
  return isset($error_description[$error_id]) ? $error_description[$error_id] : t('unknown');
}

function _commerce_payu_details_url($trans_id) {
  return 'https://www.platnosci.pl/paygw/adm/showTransDetails.do?transId=' . $trans_id;
}

