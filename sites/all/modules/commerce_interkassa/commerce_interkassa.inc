<?php

/*
 * Proccess payment
 */

function _commerce_interkassa_status() {
    $data = array('ik_shop_id' => 'C6C00382-2752-9476-9CED-3C766EA2BD10',
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
    
    //step 1 - check signature
    if (_commerce_interkassa_get_sign_hash($data)) {
        die('ok');
    } else {
        die('fail');
    }
}

function _commerce_interkassa_success() {
    //
}

function _commerce_interkassa_fail() {
    //
}

/**
 * Verifying the signature information
 */

/**
 *
 * @param type $status_data
 * @return true - Verifying the signature information of the payment is successful!, 
 *         null - Verifying the signature information about the payment failed
 */
function _commerce_interkassa_get_sign_hash($status_data) {

    $secret_key = variable_get('commerce_interkassa_secret_id');

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