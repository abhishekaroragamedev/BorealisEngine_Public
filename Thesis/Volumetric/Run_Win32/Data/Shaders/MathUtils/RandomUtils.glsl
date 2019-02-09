float PseudoRand( float seed )
{
	// From https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
	return fract( sin( seed ) * 43758.5453123f );
}
