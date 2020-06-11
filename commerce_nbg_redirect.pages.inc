<?php

/**
 * @file
 * nbg menu items.
 */

define('AB_SUCCESFUL_TRANSACTION_AUTHORIZED', 'AUTHORIZED');
define('AB_SUCCESFUL_TRANSACTION_CAPTURED', 'CAPTURED');
define('AB_ERROR_TRANSACTION', 'ERROR');
define('AB_REFUSED_TRANSACTION', 'REFUSED');
define('AB_TRANSACTION_CANCELED', 'CANCELED');

/**
 * Hook function for succesful order.
 */
function commerce_nbg_redirect_complete() {
  // Get the data from the simplify form after the redirect.
  require_once(dirname(__FILE__) ."/SimpleCrypt.inc"); 
  //require_once(dirname(__FILE__) ."/lib/Simplify.php");
    
  $crypt_key=variable_get('nbg_encrypt_secret');
  $privateKey=variable_get('nbg_private_key');
  $publicKey= variable_get('nbg_public_key');
    
  $order_id='';  
  $order=false;


  
  $data=$_REQUEST;
 
  
  $amount = (isset($data['amount']))? $data['amount']: '';
  $paymentId = (isset($data['paymentId']))? $data['paymentId']: '';
  $paymentDate = (isset($data['paymentDate']))? $data['paymentDate']: '';
  $paymentStatus = (isset($data['paymentStatus']))? $data['paymentStatus']: '';
  $authCode = (isset($data['authCode']))? $data['authCode']: '';
  $currency = (isset($data['currency']))? $data['currency']: '';
  $signature = (isset($data['signature']))? $data['signature']: '';
  $reference = (isset($data['reference']))? $data['reference']: '';

  $recreatedSignature='';
  if($paymentStatus=='APPROVED'){
  	$order_id = SimpleCrypt::decrypt($reference, $crypt_key,true);
  
    $order = commerce_static_checkout_url_load_order_or_die($order_id);
    if($order){
    	$loaded_amount=!empty($order->commerce_order_total[LANGUAGE_NONE][0]['amount']) ? $order->commerce_order_total[LANGUAGE_NONE][0]['amount']: '';
    	if($loaded_amount==$amount){
    		$recreatedSignature = strtoupper(md5($amount . $reference . $paymentId . $paymentDate . $paymentStatus . $privateKey));
    	}
    }

  } else {
  	 commerce_static_checkout_url_redirect_with_error('nbg', $order, 'Wrong POST DATA', TRUE);
  }
  if ($recreatedSignature != $signature) {
	   commerce_static_checkout_url_redirect_with_error('nbg', $order, 'Fail - signatures do not match', TRUE);
	} 

 //  $amount = (isset($data['amount']))? $data['amount']: '';
 //  $cardToken = (isset($data['paymentId']))? $data['paymentId']: '';
 //  $reference = (isset($data['reference']))? $data['reference']: '';

 //  $match = false;
 //  if( $reference != '') {
 //  	$order_id = SimpleCrypt::decrypt($reference, $crypt_key,true);
 //    $order = commerce_static_checkout_url_load_order_or_die($order_id);
 //    if($order){
 //    	$loaded_amount=!empty($order->commerce_order_total[LANGUAGE_NONE][0]['amount']) ? $order->commerce_order_total[LANGUAGE_NONE][0]['amount']: '';
 //    	if($loaded_amount==$amount){
 //    		$match = true;
 //    	}
 //    }
 //  }
 //  if ($match == false) {
	//     return 'An error occured';
	// } 

	

	// //Attempt the payment
	// Simplify::$publicKey = $publicKey;
	// Simplify::$privateKey = $privateKey;
	 
	// $payment = Simplify_Payment::createPayment(array(
	//         'amount' => $amount,
	//         'description' => 'order: '.$order_id,
	//         'token' => $cardToken,
	//         'reference' => $reference ,
	//         'currency' => 'EUR' 
	// ));
	 
	// if (!$payment->paymentStatus == 'APPROVED') {
	//     return 'Payment could not be proccessed';
	// }
	// dsm($payment);

	//return 'payment ok';


  watchdog('commerce_nbg', 'Receiving new order notification for order @order_id.', array('@order_id' => $order_id));



  $payment_status = 'APPROVED';
  $delta_pay_id =  $paymentId ;
  $payment_amount = $amount;
  $payment_currency_code =  'EUR';



  $transaction = commerce_payment_transaction_new('commerce_nbg_redirect', $order_id);
  $transaction->instance_id = $delta_pay_id;
  $transaction->amount = $payment_amount;
  $transaction->currency_code = $payment_currency_code;
  $transaction->payload[REQUEST_TIME] = $data;
  $transaction->status = 'COMMERCE_PAYMENT_STATUS_SUCCESS';
  $transaction->remote_id = $delta_pay_id;
  $transaction->remote_status = $payment_status;
  $transaction->message = t('Payment was successful');
  $result = commerce_payment_transaction_save($transaction);
  commerce_checkout_complete($order);

  commerce_static_checkout_url_redirect_with_success('nbg');


  return $result;
}

/**
 * Hook function for order with failure.
 */
function commerce_nbg_redirect_error() {
  // NOT USED

}
