#pragma once

struct MaterialData {
// sprê¿ystoœæ
	float density;
	float youngMaterial;
	float poissonRatio;

// plastycznoœæ (Johnson-Cook)
	float A;
	float B;
	float n;
	float C;
	float m;

// zniszczenie (Johnson-Cook)
	float D1;
	float D2;
	float D3;
	float D4;
	float D5;

// termomechanika
	float T_room;
	float T_melt;
	float specificHeat;
	float eps_dot_0;
	float taylorQuinney;

// hydrodynamika (equation of state)
	float eos_C0;
	float eos_S;
	float eos_Gamma0;

};