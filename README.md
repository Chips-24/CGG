# CGG

Compilation

```
make         // pour compiler tout les fichiers dans src/
    ou
make DEBUG=1 // pour avoir les symboles de debug (-g3)
```

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