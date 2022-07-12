<?php 
/**
 * Credit to http://www.php.net/manual/en/function.base64-encode.php#103849
 */
function base64url_encode($data) { 
  return rtrim(strtr(base64_encode($data), '+/', '-_'), '='); 
} 

function base64url_decode($data) { 
  return base64_decode(str_pad(strtr($data, '-_', '+/'), strlen($data) % 4, '=', STR_PAD_RIGHT)); 
} 


/**
 * Credit to http://us3.php.net/manual/en/function.mcrypt-encrypt.php#78531
 * 
 * Changed base64 to base64url 
 */
class Cipher {
    private $securekey, $iv;
    function __construct($textkey) {
        $this->securekey = hash('sha256',$textkey,TRUE);
        $this->iv = mcrypt_create_iv(32);
    }
    function encrypt($input) {
        return base64url_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $this->securekey, $input, MCRYPT_MODE_ECB, $this->iv));
    }
    function decrypt($input) {
        return trim(mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $this->securekey, base64url_decode($input), MCRYPT_MODE_ECB, $this->iv));
    }
}

?>