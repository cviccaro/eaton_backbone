<?php

module_load_include('inc', 'eaton_backbone', 'inc/lib/fpdf/MemImageFpdf.class');

/**
 * @file
 * Abstract parent class for sticker sheet stickers.
 * 
 * @author
 * Guy Paddock (gapopl@rit.edu)
 */
abstract class Sticker {
  /**
   * Render this sticker to the provided PDF renderer at the position
   * and size specified.
   * 
   * @param $pdf
   *   The FPDF writer to use for output.
   * 
   * @param $x
   *   The x position of the upper-left corner of the
   *   sticker.
   * 
   * @param $y
   *   The y position of the upper-left corner of the
   *   sticker.
   * 
   * @param $width
   *   The width of the sticker.
   * 
   * @param $height
   *   The height of the sticker.
   * 
   * @throws InvalidArgumentException
   *   If the player has not been set with setPlayer()
   *   before calling this method.
   */
  public abstract function render(MemImageFpdf $pdf, $x, $y, $width, $height);
}
