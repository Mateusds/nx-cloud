#!/bin/bash

# Script para automatizar compilação, commit, push e release
# Uso: ./push-to-develop.sh

set -e

echo "=========================================="
echo "NX-Cloud Switch - Build & Push Script"
echo "=========================================="
echo ""

# Passo 0: Compilação
echo "Passo 0: Compilando o projeto..."
cd switch-app
if make; then
    echo "✅ Compilação bem-sucedida!"
    cd ..
else
    echo "❌ Erro na compilação! O script será interrompido."
    cd ..
    exit 1
fi

echo ""
# Pergunta a mensagem de commit
read -p "Digite a mensagem do commit: " commit_msg

if [ -z "$commit_msg" ]; then
    echo "Erro: Mensagem de commit não pode ser vazia!"
    exit 1
fi

echo ""
echo "Passo 1: Adicionando arquivos..."
git add -A

echo "Passo 2: Criando commit na branch mateus..."
if [ -n "$(git status --porcelain)" ]; then
    git commit -m "$commit_msg"
else
    echo "ℹ️ Nada para commitar, pulando para o próximo passo."
fi

echo "Passo 3: Push da branch mateus para origin..."
git push origin mateus

echo ""
echo "Passo 4: Merge mateus -> develop"
git checkout develop
git pull origin develop 2>/dev/null || true
git merge mateus -m "Merge from mateus: $commit_msg"

echo "Passo 5: Push da branch develop para origin..."
git push origin develop

echo ""
echo "=========================================="
echo "✅ CÓDIGO ENVIADO COM SUCESSO!"
echo "=========================================="
echo ""

# BÔNUS: Automação de Release
read -p "Deseja criar uma Release oficial para os usuários agora? (s/N): " confirm_release

if [[ "$confirm_release" =~ ^[Ss]$ ]]; then
    echo ""
    read -p "Digite a versão (ex: v0.6.0): " version_tag
    
    if [ -z "$version_tag" ]; then
        echo "Versão inválida. Release cancelada."
    else
        echo "Criando release $version_tag no GitHub..."
        # O arquivo .nro agora fica na pasta switch-app
        if [ -f "switch-app/AppSwitch.nro" ]; then
            gh release create "$version_tag" "switch-app/AppSwitch.nro" --title "Versão $version_tag" --notes "$commit_msg"
            echo "🚀 Release criada! A API será atualizada automaticamente via GitHub Actions."
        else
            echo "❌ Erro: switch-app/AppSwitch.nro não encontrado. A release não foi criada."
        fi
    fi
fi

echo ""
echo "Passo 6: Voltando para branch mateus..."
git checkout mateus
echo "Pronto para continuar desenvolvendo!"
echo ""
