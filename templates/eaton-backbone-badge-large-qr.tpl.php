<?php
if (!function_exists('text_limit')) {
	function text_limit($text, $limit) {
		if (strlen($text) >= $limit) {
			return substr($text, 0, $limit) . '...';
		}
		return $text;
	}
}
$text_limit = variable_get('eaton_backbone_badging_text_limit', 30);
$eaton_authorized = trim(strtolower($attendee->company)) == 'eaton authorized';
$eaton_nonauth = trim(strtolower($attendee->company)) == 'eaton';

if (!function_exists('textlengthclass')) {
	function textlengthclass($string) {
		$cssClass = '';
		$strlen = strlen($string);
		switch (true) {
		case in_array($strlen, range(0, 8)):
			$cssClass = 'short';
			break;
		case in_array($strlen, range(9, 16)):
			$cssClass = 'medium';
			break;
		case in_array($strlen, range(17, 24)):
			$cssClass = 'long';
			break;
		case in_array($strlen, range(25, 200)):
			$cssClass = 'extra-long';
			break;
		}
		return $cssClass;
	}
}
if (!isset($background) || empty($background)) {
	$wrapper_attributes['class'][] = 'no-background';
}
$tclasses = array(
	'first_name' => textlengthclass($attendee->first_name),
	'last_name' => textlengthclass($attendee->first_name),
	'title' => textlengthclass($attendee->title),
	'company' => textlengthclass($attendee->company),
);
?>
<page backtop="0mm" backbottom="0mm" backleft="0mm" backright="0mm" <?php if (isset($background) && !empty($background)): ?> backimg="<?php print $background;?>" <?php endif;?>>
<table <?php print drupal_attributes($wrapper_attributes);?>>
  <tr class="empty-row"><td class="empty"></td></tr>
  <tr class="badge-large-qr">
    <td>
    <?php print theme_image(array('path' => $file->uri, 'width' => $image_width, 'height' => $image_height, 'alt' => '', 'title' => '', 'attributes' => array('class' => array('qrcode'))));?>
   </td>
 </tr>
  <tr class="badge-main">
    <td class="badge-left">
      <div class="text-length-<?php print strlen($attendee->first_name);?> text-length-<?php print $tclasses['first_name'];?> badge-attendee-first-name"><?php print $attendee->first_name;?></div>
      <div class="text-length-<?php print strlen($attendee->last_name);?> text-length-<?php print $tclasses['last_name'];?> badge-attendee-last-name"><?php print $attendee->last_name;?></div>
    </td>
  </tr>
  <tr class="badge-bottom">
    <td>
        <?php $company = $eaton_authorized ? 'Eaton' : $attendee->company;?>
        <div class="text-length-<?php print strlen($company);?> text-length-<?php print $tclasses['company'];?> badge-attendee-company"><?php print text_limit($company, $text_limit);?></div>
        <div class="text-length-<?php print strlen($attendee->title);?> text-length-<?php print $tclasses['title'];?> badge-attendee-title "><?php print text_limit($attendee->title, $text_limit);?></div>
    </td>
  </tr>
</table>
<div class="badge-ribbon badge-ribbon<?php if ($eaton_authorized) {print '-authorized';}?><?php if ($eaton_nonauth) {print '-nonauthorized';}?>"></div>
</page>