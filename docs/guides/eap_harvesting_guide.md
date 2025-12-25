# 🦅 Guia do EapHarvester (Rogue Authenticator)

O módulo **EapHarvester** (parte do conjunto "Trinity") transforma seu dispositivo T-Dongle S3 em um **falso autenticador 802.1X**. Isso permite capturar credenciais de domínio de computadores que tentam se conectar a redes cabeadas protegidas.

---

## ⚠️ Aviso Legal
Esta ferramenta é destinada exclusivamente para testes de penetração autorizados e fins educacionais. O uso não autorizado em redes corporativas é ilegal.

---

## 🎯 O Que Ele Faz?

1.  **Impersonação:** O dispositivo se comporta como um Switch corporativo legítimo.
2.  **Solicitação:** Quando a vítima (PC) se conecta via cabo USB (emulando Ethernet), o EapHarvester solicita a identidade do usuário.
3.  **Desafio:** Se a vítima responder, o EapHarvester envia um desafios MD5 malicioso.
4.  **Captura:** A resposta da vítima (hash MSCHAPv2 ou MD5) é capturada e salva.
5.  **Exfiltração:** Os hashes podem ser quebrados offline (ex: Hashcat) para revelar a senha do domínio.

---

## 🚀 Como Usar

### 1. Hardware Necessário
*   **T-Dongle S3** (com USB Army Knife Firmware instalado).
*   **Computador Vítima** configurado para autenticação 802.1X (comum em notebooks corporativos).

### 2. Ativação
Você pode controlar o EapHarvester através da interface Web do dispositivo.

1.  Conecte-se ao WiFi do T-Dongle (SSID: `USB Army Knife`, Senha: `password`).
2.  Abra o navegador e acesse `http://192.168.4.1`.
3.  Utilize os endpoints da API (futuramente haverá botões na UI):
    *   **Iniciar Ataque:** Acesse `http://192.168.4.1/trinity/eap/start`
    *   **Parar Ataque:** Acesse `http://192.168.4.1/trinity/eap/stop`
    *   **Ver Status:** Acompanhe o contador `eapCaptured` em `/data.json`.

### 3. Exemplo de Fluxo de Ataque

1.  O atacante conecta o T-Dongle na porta USB da vítima.
2.  O Windows detecta uma nova "Placa de Rede" de alta velocidade (NCM).
3.  O Windows tenta se autenticar na "rede corporativa" via 802.1X.
4.  O LED do T-Dongle pisca (indicando atividade).
5.  O atacante remove o dispositivo.
6.  O arquivo `eap_exfil.csv` no cartão SD contém: `MAC_VITIMA,DOMINIO\USUARIO,CHALLENGE,RESPONSE_HASH`.

### 4. Quebrando a Senha
Utilize o `hashcat` ou `john` para quebrar o hash capturado.
Exemplo (formato genérico):
```bash
hashcat -m 5500 captured_hash.txt wordlist.txt
```

---

## 🛡️ Defesa e Mitigação
*   Utilize validação de certificados do servidor (EAP-TLS) ao invés de apenas usuário/senha (PEAP-MSCHAPv2).
*   Configure seus clientes para validar a identidade do servidor autenticador.
*   Desabilite a negociação automática de Ethernet em portas USB se não for necessária.
