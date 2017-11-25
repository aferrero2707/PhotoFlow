# fw-public

This github repo (fw-public) consists of shared code made available for use under the Creative Commons CCO license. This code is extracted from the Filmic Worlds, LLC codebase.

# Filmic Tone Curve and Color Grading
The Filmic Tone Curve includes a filmic tonemapping curve using three segments of a power curve. The Filmic Color Grading include the tone curve as well as several operations for Exposure, Color Balance, Saturation, Contrast, and Lift/Gamma/Gain.

# Usage:
FilmicColorGrading::UserParams userParams; // User params are the input

...

FilmicColorGrading::RawParams rawParams;
FilmicColorGrading::EvalParams evalParams;
FilmicColorGrading::BakedParams bakeParams;

FilmicColorGrading::RawFromUserParams(rawParams, userParams);
FilmicColorGrading::EvalFromRawParams(evalParams,rawParams);
FilmicColorGrading::BakeFromEvalParams(bakeParams,evalParams,256,FilmicColorGrading::kTableSpacing_Quadratic);

...

Vec3 srcColor = ...;

Vec3 dstColor = bakeParams.Eval(srcColor);


