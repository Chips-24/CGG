# CGG

Pour compiler

```
cmake -S . -B build
cmake --build build/
./build/RiemannSiegel 10 1000 100
```

Pour lancer un programme vite fait avec slurm :
```
./quick.sbatch <prog> [args ...]
```

Pour comparer deux versions du programme :  
```
./compare.sbatch <id_mesurement> <nb_reps> <older_prog> <newer_prog> [args ...]
./speedup.sh <id_mesurement> [output_dir]
```  
Par défault les résultats sont dans `output/` et les sorties slurm dans `results.log` et `errors.log`.  
Pensez à définir le chemin vers vos scripts pour slurm avec la variable d'environnement `SLURM_YOUR_SCRIPT_DIR`