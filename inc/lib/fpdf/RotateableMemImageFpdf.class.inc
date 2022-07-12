<?php

/**
 * @file
 * Sub-class of MemImageFpdf that adds rotation.
 * Modified slightly to work with the qrs_sheets module.
 * Source: http://www.fpdf.org/en/script/script2.php
 *
 * @author
 * Olivier (oliver@fpdf.org)
 *
 * @license
 * FPDF
 */

// Drupal call.
module_load_include('inc', 'eaton_backbone', 'inc/lib/fpdf/MemImageFpdf.class');

class RotateableMemImageFpdf extends MemImageFpdf
{
	private $angle = 0;
	
	function SetRotation($angle, $x = -1, $y = -1)
	{
		if ($x == - 1)
			$x = $this->x;
			
		if ($y == - 1)
			$y = $this->y;
			
		if ($this->angle != 0)
			$this->_out('Q');
			
		$this->angle = $angle;
		
		if ($angle != 0)
		{
			$angle	*= M_PI / 180;
			$c		= cos ( $angle );
			$s		= sin ( $angle );
			$cx		= $x * $this->k;
			$cy		= ($this->h - $y) * $this->k;
			
			$this->_out(sprintf(
					'q %.5f %.5f %.5f %.5f %.2f %.2f cm 1 0 0 1 %.2f %.2f cm',
					$c, $s, - $s, $c, $cx, $cy, - $cx, - $cy ) );
		}
	}

	function _endpage()
	{
		// Reset rotation on new page
		if ($this->angle != 0)
		{
			$this->angle = 0;
			$this->_out('Q');
		}
		
		parent::_endpage();
	}

  /**
   * Below is from an example provided by the author.
   * See the source address in the @file declaration.
   */

	function RotatedText($x, $y, $txt, $angle)
	{
	    //Text rotated around its origin
	    $this->SetRotation($angle, $x, $y);
	    $this->Text($x, $y, $txt);
	    $this->SetRotation(0);
	}
	
	function RotatedImage($file, $x, $y, $w, $h, $angle)
	{
	    //Image rotated around its upper-left corner
	    $this->SetRotation($angle, $x, $y);
	    $this->Image($file, $x, $y, $w, $h);
	    $this->SetRotation(0);
	}

}
?>
