<?php

function _commerce_interkassa_status() {
    
}

function _commerce_interkassa_success() {
    //
}

function _commerce_interkassa_fail() {
    //
}

function _commerce_interkassa_() {
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
        echo 'Verifying the signature information of the payment is successful!';
    } else {
        echo 'Verifying the signature information about the payment failed!';
    }
}