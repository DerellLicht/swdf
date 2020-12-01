## swdf - Stalker Weapon/Armour Degradation Fix

This scans through all .ltx files in current directory, searching for various
target lines, and setting parameters to 0.0.<br>
This is the mechanism for disabling weapon/armour degradation in S.T.A.L.K.E.R. games.
Currently, this works for weapons and outfits .ltx files.

###  Example output for weapons file

        filespec: *.ltx, 52 found

        ==========================================
        original (backup) file: w_ak105.ltx.bak
        modified file: w_ak105.ltx
        condition_queue_shot_dec = 0.0 ;= 0.0 ;= 0.0 ;= 0.0 ;= 0.0008
        condition_shot_dec = 0.0 ;= 0.0 ;= 0.0 ;= 0.0 ;= 0.0008

###  Example output for armour file

        filespec: *.ltx, 22 found

        ==========================================
        original (backup) file: o_exoskeleton.ltx.bak
        modified file: o_exoskeleton.ltx
        burn_immunity = 0.0 ;= 0.04
        chemical_burn_immunity = 0.0 ;= 0.016
        explosion_immunity = 0.0 ;= 0.07
        fire_wound_immunity = 0.0 ;= 0.007
        radiation_immunity = 0.0 ;= 0.0
        shock_immunity = 0.0 ;= 0.018
        strike_immunity = 0.0 ;= 0
        telepatic_immunity = 0.0 ;= 0.0
        wound_immunity = 0.0 ;= 0.005


