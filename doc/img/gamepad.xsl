<?xml version="1.0" encoding="UTF-8" standalone="no"?>

<xsl:stylesheet version="1.0"
  xmlns="http://www.w3.org/2000/svg"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xlink="http://www.w3.org/1999/xlink">

  <xsl:template match="/gamepad">
    <svg width="1300" height="750" version="1.0">
      <g transform="translate(650,450)" style="font-size:32px;font-family:Source Sans Pro;font-weight:200">
        <xsl:apply-templates select="lt"/>
        <xsl:apply-templates select="rt"/>
        <g transform="translate(-379,-275)">
          <xsl:apply-templates select="document('gamepad.svg')/*/*" mode="copy">
            <xsl:with-param name="gamepad" select="."/>
          </xsl:apply-templates>
        </g>
        <xsl:apply-templates select="back"/>
        <xsl:apply-templates select="start"/>
        <xsl:apply-templates select="lb"/>
        <xsl:apply-templates select="rb"/>
        <xsl:apply-templates select="a"/>
        <xsl:apply-templates select="b"/>
        <xsl:apply-templates select="x"/>
        <xsl:apply-templates select="y"/>
        <xsl:apply-templates select="ls"/>
        <xsl:apply-templates select="lsb"/>
        <xsl:apply-templates select="up"/>
        <xsl:apply-templates select="down"/>
        <xsl:apply-templates select="left"/>
        <xsl:apply-templates select="right"/>
      </g>
    </svg>
  </xsl:template>

  <xsl:template match="lt">
    <g id="lt">
      <path d="M-220,-200L-300,-300H-600" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="-600" y="-304" style="text-anchor:start">
          <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="rt">
    <g id="rt">
      <path d="M220,-200L300,-300H600" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="600" y="-304" style="text-anchor:end">
          <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="back">
    <g id="back">
      <circle cx="-84" cy="-148" r="6"/>
      <path d="M-84,-148L-160,-400H-450" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="-450" y="-404" style="text-anchor:start">
        <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="start">
    <g id="start">
      <circle cx="84" cy="-148" r="6"/>
      <path d="M84,-148L160,-400H450" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="450" y="-404" style="text-anchor:end">
        <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="lb">
    <g id="lb">
      <circle cx="-220" cy="-270" r="6"/>
      <path d="M-220,-270L-260,-350H-600" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="-600" y="-354" style="text-anchor:start">
        <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="rb">
    <g id="rb">
      <circle cx="220" cy="-270" r="6"/>
      <path d="M220,-270L260,-350H600" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="600" y="-354" style="text-anchor:end">
        <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="a">
    <g id="a">
      <circle cx="216" cy="-90" r="6"/>
      <path d="M216,-90L390,-100H600" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="600" y="-104" style="text-anchor:end">
        <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="b">
    <g id="b">
      <circle cx="270" cy="-144" r="6"/>
      <path d="M270,-144L370,-149H600" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="600" y="-154" style="text-anchor:end">
        <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="x">
    <g id="x">
      <circle cx="162" cy="-144" r="6"/>
      <path d="M162,-144L350,-200H600" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="600" y="-204" style="text-anchor:end">
        <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="y">
    <g id="y">
      <circle cx="216" cy="-198" r="6"/>
      <path d="M216,-198L330,-250H600" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="600" y="-254" style="text-anchor:end">
        <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="ls">
    <g id="ls">
      <path d="M-239,-84A65,65 0,1,0 -277,-168L-350,-200H-600" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="-600" y="-204" style="text-anchor:start">
        <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="lsb">
    <g id="lsb">
      <circle cx="-216" cy="-145" r="6"/>
      <path d="M-216,-145L-370,-149H-600" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="-600" y="-154" style="text-anchor:start">
        <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="up">
    <g id="up">
      <circle cx="-109" cy="-68" r="6"/>
      <path d="M-109,-68L-380,-49H-600" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="-600" y="-54" style="text-anchor:start">
        <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="left">
    <g id="left">
      <circle cx="-154" cy="-23" r="6"/>
      <path d="M-154,-23L-390,1H-600" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="-600" y="-4" style="text-anchor:start">
        <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="right">
    <g id="right">
      <circle cx="-64" cy="-23" r="6"/>
      <path d="M-64,-23L-400,51H-600" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="-600" y="46" style="text-anchor:start">
        <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="down">
    <g id="down">
      <circle cx="-109" cy="22" r="6"/>
      <path d="M-109,22L-400,101H-600" style="fill:none;stroke:black;stroke-width:2px"/>
      <text x="-600" y="96" style="text-anchor:start">
        <xsl:value-of select="."/>
      </text>
    </g>
  </xsl:template>

  <xsl:template match="@*|node()" mode="copy">
    <xsl:param name="gamepad"/>
    <xsl:copy>
      <xsl:apply-templates select="@*[name() != 'id']" mode="copy"/>
      <xsl:apply-templates select="$gamepad" mode="style">
        <xsl:with-param name="button" select="@id"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="node()" mode="copy">
        <xsl:with-param name="gamepad" select="$gamepad"/>
      </xsl:apply-templates>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="*" mode="style">
    <xsl:param name="button"/>
    <xsl:copy-of select="*[name()=$button]/@style"/>
  </xsl:template>

</xsl:stylesheet>
