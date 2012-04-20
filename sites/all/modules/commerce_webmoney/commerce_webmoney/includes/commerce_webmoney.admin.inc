<?php

function commerce_webmoney_settings_form($settings = NULL) {
  $form = array();

  $settings = (array) $settings + array(
    'account' => array('test'),
	'purse_wmz' => '',
	'purse_wmu' => '',
	'purse_wme' => '',
	'purse_wmr' => '',
	'sim_mode' => array(0),
	'description' => '',
	'secret_key' => '',
	'log' => 0,
	'fail_message' => '',
	'success_message' => '',
	'status' => array('pending'),
  );

  $form['account'] = array(
    '#type' => 'radios',
    '#title' => t('Payment mode'),
    '#options' => array(
      'test' => ('Test account - for testing purposes only'),
      'live' => ('Live account - for processing real transactions'),
    ),
    '#default_value' => $settings['account'],
  );
  $form['purse_wmz'] = array(
    '#type' => 'textfield',
    '#title' => t('WMZ purse'),
    '#description' => t('WMZ purse (dollars)'),
    '#default_value' => $settings['purse_wmz'],
	);

  $form['purse_wmr'] = array(
    '#type' => 'textfield',
    '#title' => t('WMR purse'),
    '#description' => t('WMR purse (russian ruble)'),
    '#default_value' => $settings['purse_wmr'],		
  );
  
  $form['purse_wme'] = array(
    '#type' => 'textfield',
    '#title' => t('WME purse'),
    '#description' => t('WME purse (euro)'),
    '#default_value' => $settings['purse_wme'],		
  );

  $form['purse_wmu'] = array(
    '#type' => 'textfield',
    '#title' => t('WMU purse'),
    '#description' => t('WMU purse (ukrainian gryvna)'),
    '#default_value' => $settings['purse_wmu'],		
  );
 
  $form['sim_mode'] = array(
    '#type' => 'select',
    '#title' => t('Testing mode'),
    '#description' => t('How to simulate the execution of payment in test mode.'),
    '#options' => array(0 => t('All succesful'), 1 => t('All unsuccessful'), 2 => t('80% - succesful, 20% - unsuccessful'),),
    '#default_value' => $settings['sim_mode'],
  );

  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Order description'),
    '#description' => t('Optional description for orders that will displays in Webmoney Merchant interface. Use placeholder %order for order ID'),
    '#default_value' => $settings['description'],
  );
 
  $form['secret_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Secret key'),
    '#description' => t('Your secret key that you got from Webmoney Merchant'),
    '#default_value' => $settings['secret_key'],
  );
  
  $form['log'] = array(
    '#type' => 'select',
    '#title' => t('Log Merchant messages'),
    '#description' => t('If enabled, all merchant answers will be logged by watchdog module'),
    '#options' => array(0 => t('No'), 1 => t('Yes')),
    '#default_value' => $settings['log'],
  ); 

  $form['status'] = array(
    '#type' => 'select',
    '#title' => t('Status'),
    '#description' => t('Choose order status after customer sent the order'),
	'#options' => commerce_webmoney_statuses(),
    '#default_value' => $settings['status'],
	);

  $form['fail_message'] = array(
    '#type' => 'textarea',
	'#rows' => 3,
    '#title' => t('Fail message'),
    '#description' => t('Enter optional text that will be displayed when payment fail'),
    '#default_value' => $settings['fail_message'],
	); 

  $form['success_message'] = array(
    '#type' => 'textarea',
	'#rows' => 3,
    '#title' => t('Success message'),
    '#description' => t('Enter optional text that will be displayed when payment succesful'),
    '#default_value' => $settings['success_message'],
	);  	

  $form['#submit'][] = variable_set('commerce_webmoney_settings', $form);

  return $form;
}