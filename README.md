# Trabalho Final — Sistemas Embarcados para Veículos Aéreos

Este repositório contém o trabalho final da disciplina de Sistemas Embarcados para Veículos Aéreos da Escola de Engenharia de São Carlos (EESC-USP). A partir do [repositório-base](https://github.com/arthurUsp/Trabalho---Sistemas-Embarcados-para-Veiculos-Aereos) disponibilizado pelo professor, realizamos alterações nos códigos originais, organizadas de acordo com as contribuições de cada integrante do grupo.

O código-base é o firmware YMFC-AL de um quadricóptero com Arduino Uno.

## Integrantes

| Grupo | Integrante | Nº USP |
|---|---|---|
| AO | Ana Carolina Saggioratto Osello | 13683104 |
| AP | Ayrton Gabriel Bravo Pita | 15441914 |
| E | Esther Vieira Parise Bianchini | 15636362 |
| H | Hugo Ayello Paura | 15575883 |

Cada integrante é responsável pelas contribuições do seu grupo; a autoria de cada commit está registrada no histórico do Git.

## Contribuições

| Grupo | Área de contribuição | Documentação |
|---|---|---|
| AO | Instrumentação e sensores | [Contribuições](docs/AO.md) |
| AP | Organização do código de controle | [Contribuições](docs/AP.md) |
| E | Receptor e monitoramento da bateria | [Contribuições](docs/E.md) |
| H | Arquitetura embarcada | [Contribuições](docs/H.md) |

## Estrutura do projeto

- `YMFC-AL_setup.ino`: configura receptor e giroscópio e grava a EEPROM.
- `YMFC-AL_esc_calibrate.ino`: calibra os ESCs e testa sensores e motores.
- `YMFC-AL_Flight_controller.ino`: controlador de voo.
- `tests/receiver_diagnostics/`: diagnóstico independente do receptor, com dados simulados.

## Compilação

Cada sketch é compilado separadamente para Arduino Uno, a partir de uma pasta com o mesmo nome do arquivo:

```
arduino-cli compile --fqbn arduino:avr:uno YMFC-AL_setup
arduino-cli compile --fqbn arduino:avr:uno tests/receiver_diagnostics
```

Para executar o diagnóstico, grave-o sozinho e abra o monitor serial a 57600 bps.

## Resultados

Compilação com Arduino AVR Boards 1.8.8 (`main` original e `develop` final):

| Sketch | Flash original | Flash final | SRAM original | SRAM final |
|---|---:|---:|---:|---:|
| Flight_controller | 12374 B | 12554 B | 495 B | 501 B |
| setup | 19890 B | 20680 B | 515 B | 532 B |
| esc_calibrate | 13042 B | 13444 B | 1106 B | 1114 B |
| receiver_diagnostics | — | 4590 B | — | 506 B |

Os sketches são programas independentes; seus valores não se somam. As alterações também foram conferidas por análise do código e cálculos numéricos.
