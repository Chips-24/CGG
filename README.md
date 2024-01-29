# CGG

Compilation

Pour compiler tout les fichiers sources avec tout les 3 compilateurs :
```
make         // pour compiler avec gcc g++ et clang (sans arm)
  
make ARM=1   // pour compiler avec gcc g++ et armclang++  et activer les flags et la lib math arm
    
Ajouter DEBUG=1 pour avoir les symboles de debug (-g3)
```

Le code originel est dans `RiemannSiegel_Original.cpp`  
Le code le plus avancé est `RiemannSiegel_unpow_unroll_inline_arm_noif_table_nofmod_OMP.c`  

Pour lancer un programme vite fait avec slurm :
```
./quick.sbatch <prog> [args ...]
```
Les sorties sont dans `slurm/testout-%j.log` et `slurm/testerr-%j.log`

Pour comparer deux versions du programme :  
```
./compare.sbatch <id_mesurement> <nb_reps> <older_prog> <newer_prog> [args ...]
./speedup.sh <id_mesurement> [output_dir]
```  
Par défault les résultats sont dans `output/` et les sorties slurm dans `slurm/slurm-jobid.out` et `slurm/slurm-jobid.err`.  
Chaque mesure (définit par son id) à son propre dossier dans `output/`  

Pensez à définir `SLURM_SUBMIT_DIR` pour indiquer le dossier contenant les scripts à éxécuter.


Il y'a 3 autres scripts pour lancer les mesures :  

```
./compare_versions.sh <compiler_dir> <lower_bound> <upper_bound> <samples> [-r reps | -c core | -o]
```
This one will run every binary in compiler_dir with the given parameters.
The results are sorted by disminishing time of execution and speedup between each two consecutive binaries are dispalyed,
as well as the total speedup, between the first and the last binary.  
 The compiler_dir is one directory directly in build/  
 -r : number of repetitions for each binary  
 -c : core(s) to attach binaries to. (passed to taskset)  
 -o : overwrite error output files (Default is to append to the file)  
When not matching number of zeros found between iterations of the same binary appear, all zeros found for this binary are outputed in an error file.


```
./compare_compilers.sh <source_file> <lower_bound> <upper_bound> <samples> [-r reps | -c core | -o]"
```
This one will run every compiler in build/ for the given file and parameters.  
The source_file is one file directly in src/   
Same behaviour as compare_versions.sh  


```
./compare_two.sh <file1> <file2> <lower_bound> <upper_bound> <samples> [-r reps | -c core | -o]"
```
This one will run the two given files with the given parameters.  
Same behaviour as compare_versions.sh  