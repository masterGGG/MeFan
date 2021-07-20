<?php
/**
 * Auto generated from recommendScore.proto at 2019-07-31 11:04:34
 *
 * mifan package
 */

namespace Mifan {
/**
 * noteScore message
 */
class noteScore extends \ProtobufMessage
{
    /* Field index constants */
    const CMD = 1;
    const USER = 2;
    const ARTICLE = 3;
    const AUTHOR = 4;

    /* @var array Field descriptors */
    protected static $fields = array(
        self::CMD => array(
            'name' => 'cmd',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_INT,
        ),
        self::USER => array(
            'name' => 'user',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_INT,
        ),
        self::ARTICLE => array(
            'name' => 'article',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_INT,
        ),
        self::AUTHOR => array(
            'name' => 'author',
            'required' => false,
            'type' => \ProtobufMessage::PB_TYPE_INT,
        ),
    );

    /**
     * Constructs new message container and clears its internal state
     */
    public function __construct()
    {
        $this->reset();
    }

    /**
     * Clears message values and sets default ones
     *
     * @return null
     */
    public function reset()
    {
        $this->values[self::CMD] = null;
        $this->values[self::USER] = null;
        $this->values[self::ARTICLE] = null;
        $this->values[self::AUTHOR] = null;
    }

    /**
     * Returns field descriptors
     *
     * @return array
     */
    public function fields()
    {
        return self::$fields;
    }

    /**
     * Sets value of 'cmd' property
     *
     * @param integer $value Property value
     *
     * @return null
     */
    public function setCmd($value)
    {
        return $this->set(self::CMD, $value);
    }

    /**
     * Returns value of 'cmd' property
     *
     * @return integer
     */
    public function getCmd()
    {
        $value = $this->get(self::CMD);
        return $value === null ? (integer)$value : $value;
    }

    /**
     * Returns true if 'cmd' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasCmd()
    {
        return $this->get(self::CMD) !== null;
    }

    /**
     * Sets value of 'user' property
     *
     * @param integer $value Property value
     *
     * @return null
     */
    public function setUser($value)
    {
        return $this->set(self::USER, $value);
    }

    /**
     * Returns value of 'user' property
     *
     * @return integer
     */
    public function getUser()
    {
        $value = $this->get(self::USER);
        return $value === null ? (integer)$value : $value;
    }

    /**
     * Returns true if 'user' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasUser()
    {
        return $this->get(self::USER) !== null;
    }

    /**
     * Sets value of 'article' property
     *
     * @param integer $value Property value
     *
     * @return null
     */
    public function setArticle($value)
    {
        return $this->set(self::ARTICLE, $value);
    }

    /**
     * Returns value of 'article' property
     *
     * @return integer
     */
    public function getArticle()
    {
        $value = $this->get(self::ARTICLE);
        return $value === null ? (integer)$value : $value;
    }

    /**
     * Returns true if 'article' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasArticle()
    {
        return $this->get(self::ARTICLE) !== null;
    }

    /**
     * Sets value of 'author' property
     *
     * @param integer $value Property value
     *
     * @return null
     */
    public function setAuthor($value)
    {
        return $this->set(self::AUTHOR, $value);
    }

    /**
     * Returns value of 'author' property
     *
     * @return integer
     */
    public function getAuthor()
    {
        $value = $this->get(self::AUTHOR);
        return $value === null ? (integer)$value : $value;
    }

    /**
     * Returns true if 'author' property is set, false otherwise
     *
     * @return boolean
     */
    public function hasAuthor()
    {
        return $this->get(self::AUTHOR) !== null;
    }
}
}