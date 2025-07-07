# Deterministic Masking Evaluation of Ascon

This repository is a modified version of the [Masked Ascon Software Implementations](https://github.com/Infineon/masked-ascon) originally developed by Florian Dietrich and collaborators.

We adapted the codebase to evaluate **deterministic masking schemes** in the context of side-channel analysis. Our experiments primarily focus on:
- **First-order T-tests**
- **Second-order T-tests**

These modifications support the findings presented in our upcoming research on deterministic masking.

## Modifications and Countermeasures

Compared to the original repository, we introduced the following changes:

- **Deterministic S-box computations** without the use of fresh randomness.
- **Removal of the randomness reuse network**, ensuring the masking remains fully offline.
- **Word-level interleaving** of shares to mitigate Hamming distance and transition leakages at the hardware level.

## Usage

This implementation is designed to work with the [ChipWhisperer](https://github.com/newaetech/chipwhisperer) toolchain.

A Jupyter notebook is included in the repository to facilitate the side-channel evaluation and reproduce experimental results.

## License and Credits

The original repository and implementation were authored by:

**Florian Dietrich, Christoph Dobraunig, Florian Mendel, Robert Primas, Martin Schläffer**

Please refer to the original README for detailed documentation and relevant references.

