//=====================================================================
//	MAIN FUNCTION
//=====================================================================
void ecc(	fp timeinst,
				fp *initvalu,
				int initvalu_offset,
				fp *parameter,
				int parameter_offset,
				fp *finavalu){

	//=====================================================================
	//	VARIABLES
	//=====================================================================

	// initial data and output data variable references
	int offset_1;
	int offset_2;
	int offset_3;
	int offset_4;
	int offset_5;
	int offset_6;
	int offset_7;
	int offset_8;
	int offset_9;
	int offset_10;
	int offset_11;
	int offset_12;
	int offset_13;
	int offset_14;
	int offset_15;
	int offset_16;
	int offset_17;
	int offset_18;
	int offset_19;
	int offset_20;
	int offset_21;
	int offset_22;
	int offset_23;
	int offset_24;
	int offset_25;
	int offset_26;
	int offset_27;
	int offset_28;
	int offset_29;
	int offset_30;
	int offset_31;
	int offset_32;
	int offset_33;
	int offset_34;
	int offset_35;
	int offset_36;
	int offset_37;
	int offset_38;
	int offset_39;
	int offset_40;
	int offset_41;
	int offset_42;
	int offset_43;
	int offset_44;
	int offset_45;
	int offset_46;

	// initial data variable references
	int parameter_offset_1;

	// decoded input initial data			// GET VARIABLES FROM MEMORY AND SAVE LOCALLY !!!!!!!!!!!!!!!!!!
	fp initvalu_1;
	fp initvalu_2;
	fp initvalu_3;
	fp initvalu_4;
	fp initvalu_5;
	fp initvalu_6;
	fp initvalu_7;
	fp initvalu_8;
	fp initvalu_9;
	fp initvalu_10;
	fp initvalu_11;
	fp initvalu_12;
	fp initvalu_13;
	fp initvalu_14;
	fp initvalu_15;
	fp initvalu_16;
	fp initvalu_17;
	fp initvalu_18;
	fp initvalu_19;
	fp initvalu_20;
	fp initvalu_21;
	fp initvalu_22;
	fp initvalu_23;
	fp initvalu_24;
	fp initvalu_25;
	fp initvalu_26;
	fp initvalu_27;
	fp initvalu_28;
	fp initvalu_29;
	fp initvalu_30;
	fp initvalu_31;
	fp initvalu_32;
	fp initvalu_33;
	fp initvalu_34;
	fp initvalu_35;
	fp initvalu_36;
	fp initvalu_37;
	fp initvalu_38;
	fp initvalu_39;
	fp initvalu_40;
	fp initvalu_41;
	fp initvalu_42;
	fp initvalu_43;
	fp initvalu_44;
	fp initvalu_45;
	fp initvalu_46;

	// decoded input parameters
	fp parameter_1;

	// matlab constants undefined in c
	fp pi;

	// Constants
	fp R;																			// [J/kmol*K]
	fp Frdy;																		// [C/mol]
	fp Temp;																		// [K] 310
	fp FoRT;																		//
	fp Cmem;																		// [F] membrane capacitance
	fp Qpow;

	// Cell geometry
	fp cellLength;																	// cell length [um]
	fp cellRadius;																	// cell radius [um]
	fp junctionLength;																// junc length [um]
	fp junctionRadius;																// junc radius [um]
	fp distSLcyto;																	// dist. SL to cytosol [um]
	fp distJuncSL;																	// dist. junc to SL [um]
	fp DcaJuncSL;																	// Dca junc to SL [cm^2/sec]
	fp DcaSLcyto;																	// Dca SL to cyto [cm^2/sec]
	fp DnaJuncSL;																	// Dna junc to SL [cm^2/sec]
	fp DnaSLcyto;																	// Dna SL to cyto [cm^2/sec]
	fp Vcell;																		// [L]
	fp Vmyo;
	fp Vsr;
	fp Vsl;
	fp Vjunc;
	fp SAjunc;																		// [um^2]
	fp SAsl;																		// [um^2]
	fp J_ca_juncsl;																	// [L/msec]
	fp J_ca_slmyo;																	// [L/msec]
	fp J_na_juncsl;																	// [L/msec]
	fp J_na_slmyo;																	// [L/msec]

	// Fractional currents in compartments
	fp Fjunc;
	fp Fsl;
	fp Fjunc_CaL;
	fp Fsl_CaL;

	// Fixed ion concentrations
	fp Cli;																			// Intracellular Cl  [mM]
	fp Clo;																			// Extracellular Cl  [mM]
	fp Ko;																			// Extracellular K   [mM]
	fp Nao;																			// Extracellular Na  [mM]
	fp Cao;																			// Extracellular Ca  [mM]
	fp Mgi;																			// Intracellular Mg  [mM]

	// Nernst Potentials
	fp ena_junc;																	// [mV]
	fp ena_sl;																		// [mV]
	fp ek;																			// [mV]
	fp eca_junc;																	// [mV]
	fp eca_sl;																		// [mV]
	fp ecl;																			// [mV]

	// Na transport parameters
	fp GNa;																			// [mS/uF]
	fp GNaB;																		// [mS/uF]
	fp IbarNaK;																		// [uA/uF]
	fp KmNaip;																		// [mM]
	fp KmKo;																		// [mM]
	fp Q10NaK;
	fp Q10KmNai;

	// K current parameters
	fp pNaK;
	fp GtoSlow;																		// [mS/uF]
	fp GtoFast;																		// [mS/uF]
	fp gkp;

	// Cl current parameters
	fp GClCa;																		// [mS/uF]
	fp GClB;																		// [mS/uF]
	fp KdClCa;																		// [mM]																// [mM]

	// I_Ca parameters
	fp pNa;																			// [cm/sec]
	fp pCa;																			// [cm/sec]
	fp pK;																			// [cm/sec]
	fp KmCa;																		// [mM]
	fp Q10CaL;

	// Ca transport parameters
	fp IbarNCX;																		// [uA/uF]
	fp KmCai;																		// [mM]
	fp KmCao;																		// [mM]
	fp KmNai;																		// [mM]
	fp KmNao;																		// [mM]
	fp ksat;																			// [none]
	fp nu;																			// [none]
	fp Kdact;																		// [mM]
	fp Q10NCX;																		// [none]
	fp IbarSLCaP;																	// [uA/uF]
	fp KmPCa;																		// [mM]
	fp GCaB;																		// [uA/uF]
	fp Q10SLCaP;																	// [none]																	// [none]

	// SR flux parameters
	fp Q10SRCaP;																	// [none]
	fp Vmax_SRCaP;																	// [mM/msec] (mmol/L cytosol/msec)
	fp Kmf;																			// [mM]
	fp Kmr;																			// [mM]L cytosol
	fp hillSRCaP;																	// [mM]
	fp ks;																			// [1/ms]
	fp koCa;																		// [mM^-2 1/ms]
	fp kom;																			// [1/ms]
	fp kiCa;																		// [1/mM/ms]
	fp kim;																			// [1/ms]
	fp ec50SR;																		// [mM]

	// Buffering parameters
	fp Bmax_Naj;																	// [mM]
	fp Bmax_Nasl;																	// [mM]
	fp koff_na;																		// [1/ms]
	fp kon_na;																		// [1/mM/ms]
	fp Bmax_TnClow;																	// [mM], TnC low affinity
	fp koff_tncl;																	// [1/ms]
	fp kon_tncl;																	// [1/mM/ms]
	fp Bmax_TnChigh;																// [mM], TnC high affinity
	fp koff_tnchca;																	// [1/ms]
	fp kon_tnchca;																	// [1/mM/ms]
	fp koff_tnchmg;																	// [1/ms]
	fp kon_tnchmg;																	// [1/mM/ms]
	fp Bmax_CaM;																	// [mM], CaM buffering
	fp koff_cam;																	// [1/ms]
	fp kon_cam;																		// [1/mM/ms]
	fp Bmax_myosin;																	// [mM], Myosin buffering
	fp koff_myoca;																	// [1/ms]
	fp kon_myoca;																	// [1/mM/ms]
	fp koff_myomg;																	// [1/ms]
	fp kon_myomg;																	// [1/mM/ms]
	fp Bmax_SR;																		// [mM]
	fp koff_sr;																		// [1/ms]
	fp kon_sr;																		// [1/mM/ms]
	fp Bmax_SLlowsl;																// [mM], SL buffering
	fp Bmax_SLlowj;																	// [mM]
	fp koff_sll;																	// [1/ms]
	fp kon_sll;																		// [1/mM/ms]
	fp Bmax_SLhighsl;																// [mM]
	fp Bmax_SLhighj;																// [mM]
	fp koff_slh;																	// [1/ms]
	fp kon_slh;																		// [1/mM/ms]
	fp Bmax_Csqn;																	// 140e-3f*Vmyo/Vsr; [mM]
	fp koff_csqn;																	// [1/ms]
	fp kon_csqn;																	// [1/mM/ms]

	// I_Na: Fast Na Current
	fp am;
	fp bm;
	fp ah;
	fp bh;
	fp aj;
	fp bj;
	fp I_Na_junc;
	fp I_Na_sl;
	fp I_Na;

	// I_nabk: Na Background Current
	fp I_nabk_junc;
	fp I_nabk_sl;
	fp I_nabk;

	// I_nak: Na/K Pump Current
	fp sigma;
	fp fnak;
	fp I_nak_junc;
	fp I_nak_sl;
	fp I_nak;

	// I_kr: Rapidly Activating K Current
	fp gkr;
	fp xrss;
	fp tauxr;
	fp rkr;
	fp I_kr;

	// I_ks: Slowly Activating K Current
	fp pcaks_junc;
	fp pcaks_sl;
	fp gks_junc;
	fp gks_sl;
	fp eks;
	fp xsss;
	fp tauxs;
	fp I_ks_junc;
	fp I_ks_sl;
	fp I_ks;

	// I_kp: Plateau K current
	fp kp_kp;
	fp I_kp_junc;
	fp I_kp_sl;
	fp I_kp;

	// I_to: Transient Outward K Current (slow and fast components)
	fp xtoss;
	fp ytoss;
	fp rtoss;
	fp tauxtos;
	fp tauytos;
	fp taurtos;
	fp I_tos;

	//
	fp tauxtof;
	fp tauytof;
	fp I_tof;
	fp I_to;

	// I_ki: Time-Independent K Current
	fp aki;
	fp bki;
	fp kiss;
	fp I_ki;

	// I_ClCa: Ca-activated Cl Current, I_Clbk: background Cl Current
	fp I_ClCa_junc;
	fp I_ClCa_sl;
	fp I_ClCa;
	fp I_Clbk;

	// I_Ca: L-type Calcium Current
	fp dss;
	fp taud;
	fp fss;
	fp tauf;

	//
	fp ibarca_j;
	fp ibarca_sl;
	fp ibark;
	fp ibarna_j;
	fp ibarna_sl;
	fp I_Ca_junc;
	fp I_Ca_sl;
	fp I_Ca;
	fp I_CaK;
	fp I_CaNa_junc;
	fp I_CaNa_sl;
	fp I_CaNa;
	fp I_Catot;

	// I_ncx: Na/Ca Exchanger flux
	fp Ka_junc;
	fp Ka_sl;
	fp s1_junc;
	fp s1_sl;
	fp s2_junc;
	fp s3_junc;
	fp s2_sl;
	fp s3_sl;
	fp I_ncx_junc;
	fp I_ncx_sl;
	fp I_ncx;

	// I_pca: Sarcolemmal Ca Pump Current
	fp I_pca_junc;
	fp I_pca_sl;
	fp I_pca;

	// I_cabk: Ca Background Current
	fp I_cabk_junc;
	fp I_cabk_sl;
	fp I_cabk;

	// SR fluxes: Calcium Release, SR Ca pump, SR Ca leak
	fp MaxSR;
	fp MinSR;
	fp kCaSR;
	fp koSRCa;
	fp kiSRCa;
	fp RI;
	fp J_SRCarel;																	// [mM/ms]
	fp J_serca;
	fp J_SRleak;																		//   [mM/ms]

	// Cytosolic Ca Buffers
	fp J_CaB_cytosol;

	// Junctional and SL Ca Buffers
	fp J_CaB_junction;
	fp J_CaB_sl;

	// SR Ca Concentrations
	fp oneovervsr;

	// Sodium Concentrations
	fp I_Na_tot_junc;																// [uA/uF]
	fp I_Na_tot_sl;																	// [uA/uF]
	fp oneovervsl;

	// Potassium Concentration
	fp I_K_tot;

	// Calcium Concentrations
	fp I_Ca_tot_junc;																// [uA/uF]
	fp I_Ca_tot_sl;																	// [uA/uF]
	fp junc_sl;
	fp sl_junc;
	fp sl_myo;
	fp myo_sl;

	//	Simulation type
	int state;																			// 0-none; 1-pace; 2-vclamp
	fp I_app;
	fp V_hold;
	fp V_test;
	fp V_clamp;
	fp R_clamp;

	//	Membrane Potential
	fp I_Na_tot;																		// [uA/uF]
	fp I_Cl_tot;																		// [uA/uF]
	fp I_Ca_tot;
	fp I_tot;

	//=====================================================================
	//	EXECUTION
	//=====================================================================

	// variable references
	offset_1  = initvalu_offset;
	offset_2  = initvalu_offset+1;
	offset_3  = initvalu_offset+2;
	offset_4  = initvalu_offset+3;
	offset_5  = initvalu_offset+4;
	offset_6  = initvalu_offset+5;
	offset_7  = initvalu_offset+6;
	offset_8  = initvalu_offset+7;
	offset_9  = initvalu_offset+8;
	offset_10 = initvalu_offset+9;
	offset_11 = initvalu_offset+10;
	offset_12 = initvalu_offset+11;
	offset_13 = initvalu_offset+12;
	offset_14 = initvalu_offset+13;
	offset_15 = initvalu_offset+14;
	offset_16 = initvalu_offset+15;
	offset_17 = initvalu_offset+16;
	offset_18 = initvalu_offset+17;
	offset_19 = initvalu_offset+18;
	offset_20 = initvalu_offset+19;
	offset_21 = initvalu_offset+20;
	offset_22 = initvalu_offset+21;
	offset_23 = initvalu_offset+22;
	offset_24 = initvalu_offset+23;
	offset_25 = initvalu_offset+24;
	offset_26 = initvalu_offset+25;
	offset_27 = initvalu_offset+26;
	offset_28 = initvalu_offset+27;
	offset_29 = initvalu_offset+28;
	offset_30 = initvalu_offset+29;
	offset_31 = initvalu_offset+30;
	offset_32 = initvalu_offset+31;
	offset_33 = initvalu_offset+32;
	offset_34 = initvalu_offset+33;
	offset_35 = initvalu_offset+34;
	offset_36 = initvalu_offset+35;
	offset_37 = initvalu_offset+36;
	offset_38 = initvalu_offset+37;
	offset_39 = initvalu_offset+38;
	offset_40 = initvalu_offset+39;
	offset_41 = initvalu_offset+40;
	offset_42 = initvalu_offset+41;
	offset_43 = initvalu_offset+42;
	offset_44 = initvalu_offset+43;
	offset_45 = initvalu_offset+44;
	offset_46 = initvalu_offset+45;

	// variable references
	parameter_offset_1  = parameter_offset;

	// decoded input initial data
	initvalu_1  = initvalu[offset_1 ];
	initvalu_2  = initvalu[offset_2 ];
	initvalu_3  = initvalu[offset_3 ];
	initvalu_4  = initvalu[offset_4 ];
	initvalu_5  = initvalu[offset_5 ];
	initvalu_6  = initvalu[offset_6 ];
	initvalu_7  = initvalu[offset_7 ];
	initvalu_8  = initvalu[offset_8 ];
	initvalu_9  = initvalu[offset_9 ];
	initvalu_10 = initvalu[offset_10];
	initvalu_11 = initvalu[offset_11];
	initvalu_12 = initvalu[offset_12];
	initvalu_13 = initvalu[offset_13];
	initvalu_14 = initvalu[offset_14];
	initvalu_15 = initvalu[offset_15];
	initvalu_16 = initvalu[offset_16];
	initvalu_17 = initvalu[offset_17];
	initvalu_18 = initvalu[offset_18];
	initvalu_19 = initvalu[offset_19];
	initvalu_20 = initvalu[offset_20];
	initvalu_21 = initvalu[offset_21];
	initvalu_22 = initvalu[offset_22];
	initvalu_23 = initvalu[offset_23];
	initvalu_24 = initvalu[offset_24];
	initvalu_25 = initvalu[offset_25];
	initvalu_26 = initvalu[offset_26];
	initvalu_27 = initvalu[offset_27];
	initvalu_28 = initvalu[offset_28];
	initvalu_29 = initvalu[offset_29];
	initvalu_30 = initvalu[offset_30];
	initvalu_31 = initvalu[offset_31];
	initvalu_32 = initvalu[offset_32];
	initvalu_33 = initvalu[offset_33];
	initvalu_34 = initvalu[offset_34];
	initvalu_35 = initvalu[offset_35];
	initvalu_36 = initvalu[offset_36];
	initvalu_37 = initvalu[offset_37];
	initvalu_38 = initvalu[offset_38];
	initvalu_39 = initvalu[offset_39];
	initvalu_40 = initvalu[offset_40];
	initvalu_41 = initvalu[offset_41];
	initvalu_42 = initvalu[offset_42];
	initvalu_43 = initvalu[offset_43];
	initvalu_44 = initvalu[offset_44];
	initvalu_45 = initvalu[offset_45];
	initvalu_46 = initvalu[offset_46];

	// decoded input parameters
	parameter_1 = parameter[parameter_offset_1];

	// matlab constants undefined in c
	pi = 3.1416f;

	// Constants
	R = 8314;																			// [J/kmol*K]
	Frdy = 96485;																		// [C/mol]
	Temp = 310;																			// [K] 310
	FoRT = Frdy/R/Temp;																	//
	Cmem = 1.3810e-10f;																	// [F] membrane capacitance
	Qpow = (Temp-310)/10;

	// Cell geometry
	cellLength = 100;																	// cell length [um]
	cellRadius = 10.25f;																	// cell radius [um]
	junctionLength = 160e-3f;															// junc length [um]
	junctionRadius = 15e-3f;																// junc radius [um]
	distSLcyto = 0.45f;																	// dist. SL to cytosol [um]
	distJuncSL = 0.5f;																	// dist. junc to SL [um]
	DcaJuncSL = 1.64e-6f;																// Dca junc to SL [cm^2/sec]
	DcaSLcyto = 1.22e-6f;																// Dca SL to cyto [cm^2/sec]
	DnaJuncSL = 1.09e-5f;																// Dna junc to SL [cm^2/sec]
	DnaSLcyto = 1.79e-5f;																// Dna SL to cyto [cm^2/sec]
	Vcell = pi*powf(cellRadius,2)*cellLength*1e-15f;											// [L]
	Vmyo = 0.65f*Vcell;
	Vsr = 0.035f*Vcell;
	Vsl = 0.02f*Vcell;
	Vjunc = 0.0539f*0.01f*Vcell;
	SAjunc = 20150*pi*2*junctionLength*junctionRadius;									// [um^2]
	SAsl = pi*2*cellRadius*cellLength;													// [um^2]
	J_ca_juncsl = 1/1.2134e12f;															// [L/msec]
	J_ca_slmyo = 1/2.68510e11f;															// [L/msec]
	J_na_juncsl = 1/(1.6382e12f/3*100);													// [L/msec]
	J_na_slmyo = 1/(1.8308e10f/3*100);													// [L/msec]

	// Fractional currents in compartments
	Fjunc = 0.11f;
	Fsl = 1-Fjunc;
	Fjunc_CaL = 0.9f;
	Fsl_CaL = 1-Fjunc_CaL;

	// Fixed ion concentrations
	Cli = 15;																			// Intracellular Cl  [mM]
	Clo = 150;																			// Extracellular Cl  [mM]
	Ko = 5.4f;																			// Extracellular K   [mM]
	Nao = 140;																			// Extracellular Na  [mM]
	Cao = 1.8f;																			// Extracellular Ca  [mM]
	Mgi = 1;																			// Intracellular Mg  [mM]

	// Nernst Potentials
	ena_junc = (1/FoRT)*logf(Nao/initvalu_32);													// [mV]
	ena_sl = (1/FoRT)*logf(Nao/initvalu_33);													// [mV]
	ek = (1/FoRT)*logf(Ko/initvalu_35);														// [mV]
	eca_junc = (1/FoRT/2)*logf(Cao/initvalu_36);												// [mV]
	eca_sl = (1/FoRT/2)*logf(Cao/initvalu_37);													// [mV]
	ecl = (1/FoRT)*logf(Cli/Clo);														// [mV]

	// Na transport parameters
	GNa =  16.0f;																		// [mS/uF]
	GNaB = 0.297e-3f;																	// [mS/uF]
	IbarNaK = 1.90719f;																	// [uA/uF]
	KmNaip = 11;																		// [mM]
	KmKo = 1.5f;																			// [mM]
	Q10NaK = 1.63f;
	Q10KmNai = 1.39f;

	// K current parameters
	pNaK = 0.01833f;
	GtoSlow = 0.06f;																		// [mS/uF]
	GtoFast = 0.02f;																		// [mS/uF]
	gkp = 0.001f;

	// Cl current parameters
	GClCa = 0.109625f;																	// [mS/uF]
	GClB = 9e-3f;																		// [mS/uF]
	KdClCa = 100e-3f;																	// [mM]

	// I_Ca parameters
	pNa = 1.5e-8f;																		// [cm/sec]
	pCa = 5.4e-4f;																		// [cm/sec]
	pK = 2.7e-7f;																		// [cm/sec]
	KmCa = 0.6e-3f;																		// [mM]
	Q10CaL = 1.8f;

	// Ca transport parameters
	IbarNCX = 9.0f;																		// [uA/uF]
	KmCai = 3.59e-3f;																	// [mM]
	KmCao = 1.3f;																		// [mM]
	KmNai = 12.29f;																		// [mM]
	KmNao = 87.5f;																		// [mM]
	ksat = 0.27f;																		// [none]
	nu = 0.35f;																			// [none]
	Kdact = 0.256e-3f;																	// [mM]
	Q10NCX = 1.57f;																		// [none]
	IbarSLCaP = 0.0673f;																	// [uA/uF]
	KmPCa = 0.5e-3f;																		// [mM]
	GCaB = 2.513e-4f;																	// [uA/uF]
	Q10SLCaP = 2.35f;																	// [none]

	// SR flux parameters
	Q10SRCaP = 2.6f;																		// [none]
	Vmax_SRCaP = 2.86e-4f;																// [mM/msec] (mmol/L cytosol/msec)
	Kmf = 0.246e-3f;																		// [mM]
	Kmr = 1.7f;																			// [mM]L cytosol
	hillSRCaP = 1.787f;																	// [mM]
	ks = 25;																			// [1/ms]
	koCa = 10;																			// [mM^-2 1/ms]
	kom = 0.06f;																			// [1/ms]
	kiCa = 0.5f;																			// [1/mM/ms]
	kim = 0.005f;																		// [1/ms]
	ec50SR = 0.45f;																		// [mM]

	// Buffering parameters
	Bmax_Naj = 7.561f;																	// [mM]
	Bmax_Nasl = 1.65f;																	// [mM]
	koff_na = 1e-3f;																		// [1/ms]
	kon_na = 0.1e-3f;																	// [1/mM/ms]
	Bmax_TnClow = 70e-3f;																// [mM], TnC low affinity
	koff_tncl = 19.6e-3f;																// [1/ms]
	kon_tncl = 32.7f;																	// [1/mM/ms]
	Bmax_TnChigh = 140e-3f;																// [mM], TnC high affinity
	koff_tnchca = 0.032e-3f;																// [1/ms]
	kon_tnchca = 2.37f;																	// [1/mM/ms]
	koff_tnchmg = 3.33e-3f;																// [1/ms]
	kon_tnchmg = 3e-3f;																	// [1/mM/ms]
	Bmax_CaM = 24e-3f;																	// [mM], CaM buffering
	koff_cam = 238e-3f;																	// [1/ms]
	kon_cam = 34;																		// [1/mM/ms]
	Bmax_myosin = 140e-3f;																// [mM], Myosin buffering
	koff_myoca = 0.46e-3f;																// [1/ms]
	kon_myoca = 13.8f;																	// [1/mM/ms]
	koff_myomg = 0.057e-3f;																// [1/ms]
	kon_myomg = 0.0157f;																	// [1/mM/ms]
	Bmax_SR = 19*0.9e-3f;																	// [mM]
	koff_sr = 60e-3f;																	// [1/ms]
	kon_sr = 100;																		// [1/mM/ms]
	Bmax_SLlowsl = 37.38e-3f*Vmyo/Vsl;													// [mM], SL buffering
	Bmax_SLlowj = 4.62e-3f*Vmyo/Vjunc*0.1f;												// [mM]
	koff_sll = 1300e-3f;																	// [1/ms]
	kon_sll = 100;																		// [1/mM/ms]
	Bmax_SLhighsl = 13.35e-3f*Vmyo/Vsl;													// [mM]
	Bmax_SLhighj = 1.65e-3f*Vmyo/Vjunc*0.1f;												// [mM]
	koff_slh = 30e-3f;																	// [1/ms]
	kon_slh = 100;																		// [1/mM/ms]
	Bmax_Csqn = 2.7f;																	// 140e-3f*Vmyo/Vsr; [mM]
	koff_csqn = 65;																		// [1/ms]
	kon_csqn = 100;																		// [1/mM/ms]

	// I_Na: Fast Na Current
	fp am_arg = initvalu_39 + 47.13f;
	am = (fabsf(am_arg) < 1.0e-7f) ? 3.2f : 0.32f*am_arg/(1-expf(-0.1f*am_arg));
	bm = 0.08f*expf(-initvalu_39/11);
	if(initvalu_39 >= -40){
		ah = 0; aj = 0;
		bh = 1/(0.13f*(1+expf(-(initvalu_39+10.66f)/11.1f)));
		bj = 0.3f*expf(-2.535e-7f*initvalu_39)/(1+expf(-0.1f*(initvalu_39+32)));
	}
	else{
		ah = 0.135f*expf((80+initvalu_39)/-6.8f);
		bh = 3.56f*expf(0.079f*initvalu_39)+3.1e5f*expf(0.35f*initvalu_39);
		aj = (-127140*expf(0.2444f*initvalu_39)-3.474e-5f*expf(-0.04391f*initvalu_39))*(initvalu_39+37.78f)/(1+expf(0.311f*(initvalu_39+79.23f)));
		bj = 0.1212f*expf(-0.01052f*initvalu_39)/(1+expf(-0.1378f*(initvalu_39+40.14f)));
	}
	finavalu[offset_1] = am*(1-initvalu_1)-bm*initvalu_1;
	finavalu[offset_2] = ah*(1-initvalu_2)-bh*initvalu_2;
	finavalu[offset_3] = aj*(1-initvalu_3)-bj*initvalu_3;
	I_Na_junc = Fjunc*GNa*powf(initvalu_1,3)*initvalu_2*initvalu_3*(initvalu_39-ena_junc);
	I_Na_sl = Fsl*GNa*powf(initvalu_1,3)*initvalu_2*initvalu_3*(initvalu_39-ena_sl);
	I_Na = I_Na_junc+I_Na_sl;

	// I_nabk: Na Background Current
	I_nabk_junc = Fjunc*GNaB*(initvalu_39-ena_junc);
	I_nabk_sl = Fsl*GNaB*(initvalu_39-ena_sl);
	I_nabk = I_nabk_junc+I_nabk_sl;

	// I_nak: Na/K Pump Current
	sigma = (expf(Nao/67.3f)-1)/7;
	fnak = 1/(1+0.1245f*expf(-0.1f*initvalu_39*FoRT)+0.0365f*sigma*expf(-initvalu_39*FoRT));
	I_nak_junc = Fjunc*IbarNaK*fnak*Ko /(1+powf((KmNaip/initvalu_32),4)) /(Ko+KmKo);
	I_nak_sl = Fsl*IbarNaK*fnak*Ko /(1+powf((KmNaip/initvalu_33),4)) /(Ko+KmKo);
	I_nak = I_nak_junc+I_nak_sl;

	// I_kr: Rapidly Activating K Current
	gkr = 0.03f*sqrtf(Ko/5.4f);
	xrss = 1/(1+expf(-(initvalu_39+50)/7.5f));
	fp tauxr_arg1 = initvalu_39 + 7.0f;
	fp tauxr_arg2 = initvalu_39 + 10.0f;
	fp tauxr_term1 = (fabsf(tauxr_arg1) < 1.0e-6f)
		? 0.00138f / 0.123f
		: 0.00138f*tauxr_arg1/(1-expf(-0.123f*tauxr_arg1));
	fp tauxr_term2 = (fabsf(tauxr_arg2) < 1.0e-6f)
		? 6.1e-4f / 0.145f
		: 6.1e-4f*tauxr_arg2/(expf(0.145f*tauxr_arg2)-1);
	tauxr = 1/(tauxr_term1 + tauxr_term2);
	finavalu[offset_12] = (xrss-initvalu_12)/tauxr;
	rkr = 1/(1+expf((initvalu_39+33)/22.4f));
	I_kr = gkr*initvalu_12*rkr*(initvalu_39-ek);

	// I_ks: Slowly Activating K Current
	pcaks_junc = -log10f(initvalu_36)+3.0f;
	pcaks_sl = -log10f(initvalu_37)+3.0f;
	gks_junc = 0.07f*(0.057f +0.19f/(1+ expf((-7.2f+pcaks_junc)/0.6f)));
	gks_sl = 0.07f*(0.057f +0.19f/(1+ expf((-7.2f+pcaks_sl)/0.6f)));
	eks = (1/FoRT)*logf((Ko+pNaK*Nao)/(initvalu_35+pNaK*initvalu_34));
	xsss = 1/(1+expf(-(initvalu_39-1.5f)/16.7f));
	fp tauxs_arg = initvalu_39 + 30.0f;
	fp tauxs_term1 = (fabsf(tauxs_arg) < 1.0e-6f)
		? 7.19e-5f / 0.148f
		: 7.19e-5f*tauxs_arg/(1.0f-expf(-0.148f*tauxs_arg));
	fp tauxs_term2 = (fabsf(tauxs_arg) < 1.0e-6f)
		? 1.31e-4f / 0.0687f
		: 1.31e-4f*tauxs_arg/(expf(0.0687f*tauxs_arg)-1.0f);
	tauxs = 1.0f/(tauxs_term1 + tauxs_term2);
	finavalu[offset_13] = (xsss-initvalu_13)/tauxs;
	I_ks_junc = Fjunc*gks_junc*powf(initvalu_12,2)*(initvalu_39-eks);
	I_ks_sl = Fsl*gks_sl*powf(initvalu_13,2)*(initvalu_39-eks);
	I_ks = I_ks_junc+I_ks_sl;

	// I_kp: Plateau K current
	kp_kp = 1/(1+expf(7.488f-initvalu_39/5.98f));
	I_kp_junc = Fjunc*gkp*kp_kp*(initvalu_39-ek);
	I_kp_sl = Fsl*gkp*kp_kp*(initvalu_39-ek);
	I_kp = I_kp_junc+I_kp_sl;

	// I_to: Transient Outward K Current (slow and fast components)
	xtoss = 1/(1+expf(-(initvalu_39+3.0f)/15));
	ytoss = 1/(1+expf((initvalu_39+33.5f)/10));
	rtoss = 1/(1+expf((initvalu_39+33.5f)/10));
	tauxtos = 9/(1+expf((initvalu_39+3.0f)/15))+0.5f;
	tauytos = 3e3f/(1+expf((initvalu_39+60.0f)/10))+30;
	taurtos = 2800/(1+expf((initvalu_39+60.0f)/10))+220;
	finavalu[offset_8] = (xtoss-initvalu_8)/tauxtos;
	finavalu[offset_9] = (ytoss-initvalu_9)/tauytos;
	finavalu[offset_40]= (rtoss-initvalu_40)/taurtos;
	I_tos = GtoSlow*initvalu_8*(initvalu_9+0.5f*initvalu_40)*(initvalu_39-ek);									// [uA/uF]

	//
	tauxtof = 3.5f*expf(-initvalu_39*initvalu_39/30/30)+1.5f;
	tauytof = 20.0f/(1+expf((initvalu_39+33.5f)/10))+20.0f;
	finavalu[offset_10] = (xtoss-initvalu_10)/tauxtof;
	finavalu[offset_11] = (ytoss-initvalu_11)/tauytof;
	I_tof = GtoFast*initvalu_10*initvalu_11*(initvalu_39-ek);
	I_to = I_tos + I_tof;

	// I_ki: Time-Independent K Current
	aki = 1.02f/(1+expf(0.2385f*(initvalu_39-ek-59.215f)));
	bki =(0.49124f*expf(0.08032f*(initvalu_39+5.476f-ek)) + expf(0.06175f*(initvalu_39-ek-594.31f))) /(1 + expf(-0.5143f*(initvalu_39-ek+4.753f)));
	kiss = aki/(aki+bki);
	I_ki = 0.9f*sqrtf(Ko/5.4f)*kiss*(initvalu_39-ek);

	// I_ClCa: Ca-activated Cl Current, I_Clbk: background Cl Current
	I_ClCa_junc = Fjunc*GClCa/(1+KdClCa/initvalu_36)*(initvalu_39-ecl);
	I_ClCa_sl = Fsl*GClCa/(1+KdClCa/initvalu_37)*(initvalu_39-ecl);
	I_ClCa = I_ClCa_junc+I_ClCa_sl;
	I_Clbk = GClB*(initvalu_39-ecl);

	// I_Ca: L-type Calcium Current
	dss = 1/(1+expf(-(initvalu_39+14.5f)/6.0f));
	taud = dss*(1-expf(-(initvalu_39+14.5f)/6.0f))/(0.035f*(initvalu_39+14.5f));
	fss = 1/(1+expf((initvalu_39+35.06f)/3.6f))+0.6f/(1+expf((50-initvalu_39)/20));
	tauf = 1/(0.0197f*expf(-powf(0.0337f*(initvalu_39+14.5f),2))+0.02f);
	finavalu[offset_4] = (dss-initvalu_4)/taud;
	finavalu[offset_5] = (fss-initvalu_5)/tauf;
	finavalu[offset_6] = 1.7f*initvalu_36*(1-initvalu_6)-11.9e-3f*initvalu_6;											// fCa_junc
	finavalu[offset_7] = 1.7f*initvalu_37*(1-initvalu_7)-11.9e-3f*initvalu_7;											// fCa_sl

	//
	fp v_scaled = initvalu_39 * FoRT;
	if (fabsf(v_scaled) < 1.0e-6f) {
		ibarca_j = pCa*2.0f*Frdy*0.341f*(initvalu_36-Cao);
		ibarca_sl = pCa*2.0f*Frdy*0.341f*(initvalu_37-Cao);
		ibark = pK*Frdy*0.75f*(initvalu_35-Ko);
		ibarna_j = pNa*Frdy*0.75f*(initvalu_32-Nao);
		ibarna_sl = pNa*Frdy*0.75f*(initvalu_33-Nao);
	} else {
		fp exp_2v = expf(2.0f*v_scaled);
		fp exp_v = expf(v_scaled);
		ibarca_j = pCa*4*(initvalu_39*Frdy*FoRT) * (0.341f*initvalu_36*exp_2v-0.341f*Cao) /(exp_2v-1.0f);
		ibarca_sl = pCa*4*(initvalu_39*Frdy*FoRT) * (0.341f*initvalu_37*exp_2v-0.341f*Cao) /(exp_2v-1.0f);
		ibark = pK*(initvalu_39*Frdy*FoRT)*(0.75f*initvalu_35*exp_v-0.75f*Ko) /(exp_v-1.0f);
		ibarna_j = pNa*(initvalu_39*Frdy*FoRT) *(0.75f*initvalu_32*exp_v-0.75f*Nao)  /(exp_v-1.0f);
		ibarna_sl = pNa*(initvalu_39*Frdy*FoRT) *(0.75f*initvalu_33*exp_v-0.75f*Nao)  /(exp_v-1.0f);
	}
	I_Ca_junc = (Fjunc_CaL*ibarca_j*initvalu_4*initvalu_5*(1-initvalu_6)*powf(Q10CaL,Qpow))*0.45f;
	I_Ca_sl = (Fsl_CaL*ibarca_sl*initvalu_4*initvalu_5*(1-initvalu_7)*powf(Q10CaL,Qpow))*0.45f;
	I_Ca = I_Ca_junc+I_Ca_sl;
	finavalu[offset_43]=-I_Ca*Cmem/(Vmyo*2*Frdy)*1e3f;
	I_CaK = (ibark*initvalu_4*initvalu_5*(Fjunc_CaL*(1-initvalu_6)+Fsl_CaL*(1-initvalu_7))*powf(Q10CaL,Qpow))*0.45f;
	I_CaNa_junc = (Fjunc_CaL*ibarna_j*initvalu_4*initvalu_5*(1-initvalu_6)*powf(Q10CaL,Qpow))*0.45f;
	I_CaNa_sl = (Fsl_CaL*ibarna_sl*initvalu_4*initvalu_5*(1-initvalu_7)*powf(Q10CaL,Qpow))*0.45f;
	I_CaNa = I_CaNa_junc+I_CaNa_sl;
	I_Catot = I_Ca+I_CaK+I_CaNa;

	// I_ncx: Na/Ca Exchanger flux
	Ka_junc = 1/(1+powf((Kdact/initvalu_36),3));
	Ka_sl = 1/(1+powf((Kdact/initvalu_37),3));
	s1_junc = expf(nu*initvalu_39*FoRT)*powf(initvalu_32,3)*Cao;
	s1_sl = expf(nu*initvalu_39*FoRT)*powf(initvalu_33,3)*Cao;
	s2_junc = expf((nu-1)*initvalu_39*FoRT)*powf(Nao,3)*initvalu_36;
	s3_junc = (KmCai*powf(Nao,3)*(1+powf((initvalu_32/KmNai),3))+powf(KmNao,3)*initvalu_36+ powf(KmNai,3)*Cao*(1+initvalu_36/KmCai)+KmCao*powf(initvalu_32,3)+powf(initvalu_32,3)*Cao+powf(Nao,3)*initvalu_36)*(1+ksat*expf((nu-1)*initvalu_39*FoRT));
	s2_sl = expf((nu-1)*initvalu_39*FoRT)*powf(Nao,3)*initvalu_37;
	s3_sl = (KmCai*powf(Nao,3)*(1+powf((initvalu_33/KmNai),3)) + powf(KmNao,3)*initvalu_37+powf(KmNai,3)*Cao*(1+initvalu_37/KmCai)+KmCao*powf(initvalu_33,3)+powf(initvalu_33,3)*Cao+powf(Nao,3)*initvalu_37)*(1+ksat*expf((nu-1)*initvalu_39*FoRT));
	I_ncx_junc = Fjunc*IbarNCX*powf(Q10NCX,Qpow)*Ka_junc*(s1_junc-s2_junc)/s3_junc;
	I_ncx_sl = Fsl*IbarNCX*powf(Q10NCX,Qpow)*Ka_sl*(s1_sl-s2_sl)/s3_sl;
	I_ncx = I_ncx_junc+I_ncx_sl;
	finavalu[offset_45]=2*I_ncx*Cmem/(Vmyo*2*Frdy)*1e3f;

	// I_pca: Sarcolemmal Ca Pump Current
	I_pca_junc = 	Fjunc *
					powf(Q10SLCaP,Qpow) *
					IbarSLCaP *
					powf(initvalu_36,1.6f) /
					(powf(KmPCa,1.6f) + powf(initvalu_36,1.6f));
	I_pca_sl = 	Fsl *
				powf(Q10SLCaP,Qpow) *
				IbarSLCaP *
				powf(initvalu_37,1.6f) /
				(powf(KmPCa,1.6f) + powf(initvalu_37,1.6f));
	I_pca = I_pca_junc+I_pca_sl;
	finavalu[offset_44]=-I_pca*Cmem/(Vmyo*2*Frdy)*1e3f;

	// I_cabk: Ca Background Current
	I_cabk_junc = Fjunc*GCaB*(initvalu_39-eca_junc);
	I_cabk_sl = Fsl*GCaB*(initvalu_39-eca_sl);
	I_cabk = I_cabk_junc+I_cabk_sl;
	finavalu[offset_46]=-I_cabk*Cmem/(Vmyo*2*Frdy)*1e3f;

	// SR fluxes: Calcium Release, SR Ca pump, SR Ca leak
	MaxSR = 15;
	MinSR = 1;
	kCaSR = MaxSR - (MaxSR-MinSR)/(1+powf(ec50SR/initvalu_31,2.5f));
	koSRCa = koCa/kCaSR;
	kiSRCa = kiCa*kCaSR;
	RI = 1-initvalu_14-initvalu_15-initvalu_16;
	finavalu[offset_14] = (kim*RI-kiSRCa*initvalu_36*initvalu_14)-(koSRCa*powf(initvalu_36,2)*initvalu_14-kom*initvalu_15);			// R
	finavalu[offset_15] = (koSRCa*powf(initvalu_36,2)*initvalu_14-kom*initvalu_15)-(kiSRCa*initvalu_36*initvalu_15-kim*initvalu_16);			// O
	finavalu[offset_16] = (kiSRCa*initvalu_36*initvalu_15-kim*initvalu_16)-(kom*initvalu_16-koSRCa*powf(initvalu_36,2)*RI);			// I
	J_SRCarel = ks*initvalu_15*(initvalu_31-initvalu_36);													// [mM/ms]
	J_serca = powf(Q10SRCaP,Qpow)*Vmax_SRCaP*(powf((initvalu_38/Kmf),hillSRCaP)-powf((initvalu_31/Kmr),hillSRCaP))
										 /(1+powf((initvalu_38/Kmf),hillSRCaP)+powf((initvalu_31/Kmr),hillSRCaP));
	J_SRleak = 5.348e-6f*(initvalu_31-initvalu_36);													//   [mM/ms]

	// Sodium and Calcium Buffering
	finavalu[offset_17] = kon_na*initvalu_32*(Bmax_Naj-initvalu_17)-koff_na*initvalu_17;								// NaBj      [mM/ms]
	finavalu[offset_18] = kon_na*initvalu_33*(Bmax_Nasl-initvalu_18)-koff_na*initvalu_18;							// NaBsl     [mM/ms]

	// Cytosolic Ca Buffers
	finavalu[offset_19] = kon_tncl*initvalu_38*(Bmax_TnClow-initvalu_19)-koff_tncl*initvalu_19;						// TnCL      [mM/ms]
	finavalu[offset_20] = kon_tnchca*initvalu_38*(Bmax_TnChigh-initvalu_20-initvalu_21)-koff_tnchca*initvalu_20;			// TnCHc     [mM/ms]
	finavalu[offset_21] = kon_tnchmg*Mgi*(Bmax_TnChigh-initvalu_20-initvalu_21)-koff_tnchmg*initvalu_21;				// TnCHm     [mM/ms]
	finavalu[offset_22] = 0;																		// CaM       [mM/ms]
	finavalu[offset_23] = kon_myoca*initvalu_38*(Bmax_myosin-initvalu_23-initvalu_24)-koff_myoca*initvalu_23;				// Myosin_ca [mM/ms]
	finavalu[offset_24] = kon_myomg*Mgi*(Bmax_myosin-initvalu_23-initvalu_24)-koff_myomg*initvalu_24;				// Myosin_mg [mM/ms]
	finavalu[offset_25] = kon_sr*initvalu_38*(Bmax_SR-initvalu_25)-koff_sr*initvalu_25;								// SRB       [mM/ms]
	J_CaB_cytosol = finavalu[offset_19] + finavalu[offset_20] + finavalu[offset_21] + finavalu[offset_22] + finavalu[offset_23] + finavalu[offset_24] + finavalu[offset_25];

	// Junctional and SL Ca Buffers
	finavalu[offset_26] = kon_sll*initvalu_36*(Bmax_SLlowj-initvalu_26)-koff_sll*initvalu_26;						// SLLj      [mM/ms]
	finavalu[offset_27] = kon_sll*initvalu_37*(Bmax_SLlowsl-initvalu_27)-koff_sll*initvalu_27;						// SLLsl     [mM/ms]
	finavalu[offset_28] = kon_slh*initvalu_36*(Bmax_SLhighj-initvalu_28)-koff_slh*initvalu_28;						// SLHj      [mM/ms]
	finavalu[offset_29] = kon_slh*initvalu_37*(Bmax_SLhighsl-initvalu_29)-koff_slh*initvalu_29;						// SLHsl     [mM/ms]
	J_CaB_junction = finavalu[offset_26]+finavalu[offset_28];
	J_CaB_sl = finavalu[offset_27]+finavalu[offset_29];

	// SR Ca Concentrations
	finavalu[offset_30] = kon_csqn*initvalu_31*(Bmax_Csqn-initvalu_30)-koff_csqn*initvalu_30;						// Csqn      [mM/ms]
	oneovervsr = 1/Vsr;
	finavalu[offset_31] = J_serca*Vmyo*oneovervsr-(J_SRleak*Vmyo*oneovervsr+J_SRCarel)-finavalu[offset_30];   // Ca_sr     [mM/ms] %Ratio 3 leak current

	// Sodium Concentrations
	I_Na_tot_junc = I_Na_junc+I_nabk_junc+3*I_ncx_junc+3*I_nak_junc+I_CaNa_junc;		// [uA/uF]
	I_Na_tot_sl = I_Na_sl+I_nabk_sl+3*I_ncx_sl+3*I_nak_sl+I_CaNa_sl;					// [uA/uF]
	finavalu[offset_32] = -I_Na_tot_junc*Cmem/(Vjunc*Frdy)+J_na_juncsl/Vjunc*(initvalu_33-initvalu_32)-finavalu[offset_17];
	oneovervsl = 1/Vsl;
	finavalu[offset_33] = -I_Na_tot_sl*Cmem*oneovervsl/Frdy+J_na_juncsl*oneovervsl*(initvalu_32-initvalu_33)+J_na_slmyo*oneovervsl*(initvalu_34-initvalu_33)-finavalu[offset_18];
	finavalu[offset_34] = J_na_slmyo/Vmyo*(initvalu_33-initvalu_34);											// [mM/msec]

	// Potassium Concentration
	I_K_tot = I_to+I_kr+I_ks+I_ki-2*I_nak+I_CaK+I_kp;									// [uA/uF]
	finavalu[offset_35] = 0;															// [mM/msec]

	// Calcium Concentrations
	I_Ca_tot_junc = I_Ca_junc+I_cabk_junc+I_pca_junc-2*I_ncx_junc;						// [uA/uF]
	I_Ca_tot_sl = I_Ca_sl+I_cabk_sl+I_pca_sl-2*I_ncx_sl;								// [uA/uF]
	finavalu[offset_36] = -I_Ca_tot_junc*Cmem/(Vjunc*2*Frdy)+J_ca_juncsl/Vjunc*(initvalu_37-initvalu_36)
	         - J_CaB_junction+(J_SRCarel)*Vsr/Vjunc+J_SRleak*Vmyo/Vjunc;				// Ca_j
	finavalu[offset_37] = -I_Ca_tot_sl*Cmem/(Vsl*2*Frdy)+J_ca_juncsl/Vsl*(initvalu_36-initvalu_37)
	         + J_ca_slmyo/Vsl*(initvalu_38-initvalu_37)-J_CaB_sl;									// Ca_sl
	finavalu[offset_38] = -J_serca-J_CaB_cytosol +J_ca_slmyo/Vmyo*(initvalu_37-initvalu_38);
	junc_sl=J_ca_juncsl/Vsl*(initvalu_36-initvalu_37);
	sl_junc=J_ca_juncsl/Vjunc*(initvalu_37-initvalu_36);
	sl_myo=J_ca_slmyo/Vsl*(initvalu_38-initvalu_37);
	myo_sl=J_ca_slmyo/Vmyo*(initvalu_37-initvalu_38);

	// Simulation type
	state = 1;
	switch(state){
		case 0:
			I_app = 0;
			break;
		case 1:																			// pace w/ current injection at cycleLength 'cycleLength'
			if(fmodf(timeinst,parameter_1) <= 5){
				I_app = 9.5f;
			}
			else{
				I_app = 0.0f;
			}
			break;
		case 2:
			V_hold = -55;
			V_test = 0;
			if(timeinst>0.5f & timeinst<200.5f){
				V_clamp = V_test;
			}
			else{
				V_clamp = V_hold;
			}
			R_clamp = 0.04f;
			I_app = (V_clamp-initvalu_39)/R_clamp;
			break;
	}

	// Membrane Potential
	I_Na_tot = I_Na_tot_junc + I_Na_tot_sl;												// [uA/uF]
	I_Cl_tot = I_ClCa+I_Clbk;															// [uA/uF]
	I_Ca_tot = I_Ca_tot_junc+I_Ca_tot_sl;
	I_tot = I_Na_tot+I_Cl_tot+I_Ca_tot+I_K_tot;
	finavalu[offset_39] = -(I_tot-I_app);

	// Set unused output values to 0 (MATLAB does it by default)
	finavalu[offset_41] = 0;
	finavalu[offset_42] = 0;

}
