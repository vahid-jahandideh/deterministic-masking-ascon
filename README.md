# Deterministic Masking Evaluation of Ascon

This repository is a modified version of the [Masked Ascon Software Implementations](https://github.com/Infineon/masked-ascon) originally developed by Florian Dietrich and collaborators.

We adapted the implementation to evaluate **deterministic masking schemes** in the context of side-channel analysis. Our experiments focus on performing:
- **First-order T-tests**
- **Second-order T-tests**

These modifications support the research described in our upcoming paper on deterministic masking.

## Usage

This version is designed to be used with the [ChipWhisperer](https://github.com/newaetech/chipwhisperer) toolchain. A Jupyter notebook is provided to run leakage assessments and reproduce our results.

## License and Credits

The original repository and implementation were authored by:

Florian Dietrich, Christoph Dobraunig, Florian Mendel, Robert Primas, Martin Schläffer

Please refer to the original README for full documentation and references.

