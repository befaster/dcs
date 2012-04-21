<?php

/*
 * Proccess payment
 */
function _commerce_interkassa_status() {
    file_put_contents('status.txt', print_r($_POST, true));
    /*
    $_POST = array('ik_shop_id' => 'C6C00382-2752-9476-9CED-3C766EA2BD10',
        'ik_payment_amount' => '0.10',
        'ik_payment_id' => 'id1',
        'ik_payment_desc' => 'description',
        'ik_paysystem_alias' => 'liqpaycardu',
        'ik_baggage_fields' => '',
        'ik_payment_timestamp' => '1334927376',
        'ik_payment_state' => 'success',
        'ik_trans_id' => 'IK_6179794',
        'ik_currency_exch' => '1',
        'ik_fees_payer' => '1',
        'ik_sign_hash' => '0EE07B000658B90CCA6B6520C9C57721'
    );
    */
    //step 1 - check signature
    if (_commerce_interkassa_get_sign_hash($_POST)) {
        //step 2 - check the amount of payment
        if (!empty($_POST['ik_payment_id'])) {
            
            $order = commerce_order_load($_POST['ik_payment_id']);
            
            $payment_method_instance_id = $order->data['payment_method'];
            $payment_method = commerce_payment_method_instance_load($payment_method_instance_id);
            
            if ($order && $_POST['ik_payment_id'] == $order->order_id) {
                $wrapper = entity_metadata_wrapper('commerce_order', $order);

                $currency_code = $wrapper->commerce_order_total->currency_code->value();
                $amount = $wrapper->commerce_order_total->amount->value();
                $amount = round(commerce_currency_amount_to_decimal($amount, $currency_code), 2);

                if (!empty($_POST['ik_payment_amount']) && $_POST['ik_payment_amount'] == $amount) {
                    //step 3 - check the Shop ID in Interkassa system
                    $shop_id = _commerce_interkassa_get_settings('commerce_interkassa_shop_id');

                    if (!empty($_POST['ik_shop_id']) && ($_POST['ik_shop_id'] == $shop_id)) {
                        //step 4 - check status of paymentss
                        switch ($_POST['ik_payment_state']) {
                            case 'success':
                                commerce_interkassa_transaction($payment_method, $order, $_POST);
                                break;
                            case 'fail':
                                watchdog('commerce_interkassa', 'IPN processed: Payment status is fail', null, WATCHDOG_INFO);
                                break;
                            default:
                                watchdog('commerce_interkassa', 'IPN processed: Payment status "@payment_state" is invalid', array('@payment_state' => $_POST['ik_payment_state']), WATCHDOG_INFO);
                                break;
                        }
                    } else {
                        watchdog('commerce_interkassa', 'IPN processed: Shop ID "@shop_id" is incorrect', array('@shop_id' => $_POST['ik_shop_id']), WATCHDOG_INFO);
                    }
                } else {
                    watchdog('commerce_interkassa', 'IPN processed: Amount "@amount" is incorrect', array('@amount' => $_POST['ik_payment_amount']), WATCHDOG_INFO);
                }
            } else {
                watchdog('commerce_interkassa', 'IPN processed: Order "@order_number" doesn\'t exists.', array('@order_number' => $_POST['ik_payment_id']), WATCHDOG_INFO);
            }
        } else {
            watchdog('commerce_interkassa', 'IPN processed: Bad "@order_number" order ID.', array('@order_number' => $_POST['ik_payment_id']), WATCHDOG_INFO);
        }
    } else {        
        watchdog('commerce_interkassa', 'IPN processed: Signature "@sign_hash"  was failed.', array('@sign_hash' => $_POST['ik_sign_hash']), WATCHDOG_INFO);
    }
}

/**
 * Success page 
 */
function _commerce_interkassa_success() {
    //
}

/**
 * Fail page 
 */
function _commerce_interkassa_fail() {
    //
}

/**
 * Check signature hash
 * 
 * @param type $status_data
 * @return true - Verifying the signature information of the payment is successful!, 
 *         null - Verifying the signature information about the payment failed
 */
function _commerce_interkassa_get_sign_hash($status_data) {
    $secret_key = _commerce_interkassa_get_settings('commerce_interkassa_secret_id');

    if (!empty($status_data['ik_sign_hash']) && $secret_key) {
        $sing_hash_str = $status_data['ik_shop_id'] . ':' .
                $status_data['ik_payment_amount'] . ':' .
                $status_data['ik_payment_id'] . ':' .
                $status_data['ik_paysystem_alias'] . ':' .
                $status_data['ik_baggage_fields'] . ':' .
                $status_data['ik_payment_state'] . ':' .
                $status_data['ik_trans_id'] . ':' .
                $status_data['ik_currency_exch'] . ':' .
                $status_data['ik_fees_payer'] . ':' .
                $secret_key;

        $sign_hash = strtoupper(md5($sing_hash_str));

        if ($status_data['ik_sign_hash'] === $sign_hash) {
            return true;
        }
    }
}