---
date: 2015-04-24T10:05:29-05:00
title: Color management made easy
sub-title: Color management made easy

lede-img: 'http://4.bp.blogspot.com/-2jIUR-RdNqg/Vdc-TGx_KHI/AAAAAAAABRM/BkO93YIolZQ/s1600/Screen%2BShot%2B2015-08-21%2Bat%2B17.04.34.png'
lede-attribution: "<a href='http://photoflowblog.blogspot.fr'>Andrea Ferrero</a>"

author: "Andrea Ferrero"
author-bio: "Husband, father, scientist and spare-time photographer"
author-img: ''

layout: article
---
<table style="background-color:#101010; color=#D0D0D0">
<tr>
<td>Features</td><td>Download</td>
</tr>
</table>

# Introduction
I know it's not technical, but I can't help feel that a gentle introduction would help the average reader understand better what they're about to get into... :)

Possibly consider answering these primary questions:

*Why*
Why should the average photographer care about color management?

*What*
What is it in relation to what they're accustomed to?  What does it mean to manage color?  etc...

*Who*
Who should really care about color management?  Everyone (I know *everyone* is a likely answer)?  Practically speaking, who would benefit the most and why (see above)?

These are just the basics, but they're something important as a lede to entice the reader to actually read through the material you're about to hit them over the head with (and believe me - some of them will feel this way).

# RGB is not a Color

Show examples of the same colors in different color spaces, and show why ICC profiles are needed to correctly "interpret" RGB values

Further reading: http://www.cambridgeincolour.com/tutorials/color-spaces.htm

# The GAMUT: how much color do you need?

Explain what the GAMUT of a given colorspace is, why a wider gamut is sometimes better and sometimes dangerous.

Make practical examples of which colorspace to use when?

# Middle gray is not middle density: the origin of gamma encoding

Explain why gamma encoding was introduced to reflect the way the human vision system perceive dark and light tones.

Show the impact of different gamma encodings in different colorspaces on the practical aspects of photo editing. For example, the same RGB curve does not give the same output in sRGB and ProPhotoRGB colorpsaces...

Further reading: http://www.cambridgeincolour.com/tutorials/gamma-correction.htm

# From color to grayscale: the influence of the RGB colorspace on grayscale conversion

Show how individual RGB channels, usually mixed with varying proportions to convert the image to grayscale, look quite different in various colorspaces. Therefore, the result of the grayscale conversion depends on the input RGB colorspace...
