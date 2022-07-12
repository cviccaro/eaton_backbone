<?php

module_load_include('inc', 'eaton_backbone', 'inc/lib/fpdf/RotateableMemImageFpdf.class');

/**
 * @file
 * Abstract parent class for sticker sheets -- the sheets of stickers that
 * players exchange.
 * 
 * @author
 * Guy Paddock (gapopl@rit.edu)
 */
abstract class StickerSheet {
  /**
   * The number of rows in this sticker sheet.
   * 
   * @var int
   */
  protected $numRows;

  /**
   * The number of columns in this sticker sheet.
   * 
   * @var int
   */
  protected $numColumns;

  /**
   * All the stickers on this sheet.
   * 
   * @var  Sticker[][]
   */
  protected $stickers;

  /**
   * The text to display in the header region of this sheet.
   * 
   * @var string
   */
  protected $headerText;

  /**
   * The text to display in the footer region of this sheet.
   *
   * @var unknown_type
   */
  protected $footerText;

  /**
   * Initialize a new sticker sheet with the provided sticker content.
   * 
   * @param $numRows
   *   The number of rows in this sticker sheet.
   * 
   * @param $numColumns
   *   The number of columns in this sticker sheet.
   * 
   * @param $stickers
   *   All the stickers on this sheet.
   */
  public function StickerSheet($numRows, $numColumns, $stickers = NULL) {
    if ($numRows <= 0) {
      throw new InvalidArgumentException('$numRows must be greater than zero.');
    }

    if ($numColumns <= 0) {
      throw new InvalidArgumentException('$numColumns must be greater than zero.');
    }

    $this->numRows    = $numRows;
    $this->numColumns = $numColumns;

    if ($stickers != NULL) {
      $this->setStickers($stickers);
    }

    $this->headerText = NULL;
    $this->footerText = NULL;
  }

  /**
   * Return the number of rows in this sticker sheet.
   * 
   * @return
   */
  public function getNumRows() {
    return $this->numRows;
  }

  /**
   * Return the number of columns in this sticker sheet.
   * 
   * @return
   */
  public function getNumColumns() {
    return $this->numColumns;
  }

  /**
   * Set the text to display in the header region of this sheet.
   * 
   * @param $headerText
   *   The text to display in the header region of this sheet.
   */
  public function setHeaderText($headerText) {
    $this->headerText = $headerText;
  }

  /**
   * Set the text to display in the footer region of this sheet.
   * 
   * @param $footerText
   *   The text to display in the footer region of this sheet.
   */
  public function setFooterText($footerText) {
    $this->footerText = $footerText;
  }

  /**
   * Set all stickers on this sheet to the provided set of stickers.
   * 
   * @param $stickers
   *   All the stickers on this sheet.
   */
  public function setStickers($stickers) {
    if (count($stickers) != $this->numRows) {
      throw new InvalidArgumentException(
          '$stickers does not contain the required number of rows (' . $numRows .').');
    }

    if (count($stickers[0]) != $this->numColumns) {
      throw new InvalidArgumentException(
          '$stickers does not contain the required number of columns (' . $numColumns .').');
    }

    $this->stickers = $stickers;
  }

  /**
   * Set the sticker at the specified position to the specified value.
   *
   * @param $row
   *   The row of the sticker that is to be set.
   * 
   * @param $column
   *   The column of the sticker that is to be set.
   * 
   * @param $sticker
   *   The sticker to place at the specified position.
   */
  public function setStickerAt($row, $column, $sticker) {
    $this->stickers[$row][$column]  = $sticker;
  }

  /**
   * Render this sticker sheet to the specified PDF writer.
   * 
   * @param $pdf
   *   The FPDF instance to use for output.
   */
  public abstract function renderTo(RotateableMemImageFpdf $pdf);
}
