#!/usr/bin/env python3
# frick-fsw - Provenance attestation for evidence archives
#
# Creates a signed attestation for an evidence archive directory.
# The attestation covers the complete file manifest with SHA-256 hashes,
# the archive identity, and the creation timestamp.
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

import argparse
import hashlib
import json
import os
import subprocess
import sys
from datetime import datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def compute_file_hash(path: Path) -> str:
    """Compute SHA-256 hash of a file."""
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest()


def collect_manifest(evidence_dir: Path) -> dict:
    """Collect file manifest with hashes for all files except assets.sha256 and attestation.json."""
    manifest = {}
    for path in sorted(evidence_dir.rglob("*")):
        if path.is_file() and path.name not in ("assets.sha256", "attestation.json", "attestation.sig"):
            rel = path.relative_to(evidence_dir)
            manifest[str(rel)] = compute_file_hash(path)
    return manifest


def get_git_commit() -> str:
    """Get current git commit hash."""
    try:
        result = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            cwd=ROOT,
            capture_output=True,
            text=True,
            check=True,
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError:
        return "unknown"


def get_git_dirty() -> bool:
    """Check if working tree has uncommitted changes."""
    try:
        result = subprocess.run(
            ["git", "status", "--porcelain"],
            cwd=ROOT,
            capture_output=True,
            text=True,
            check=True,
        )
        return bool(result.stdout.strip())
    except subprocess.CalledProcessError:
        return True


def find_signing_key() -> str | None:
    """Find an SSH key suitable for signing (ed25519 or rsa)."""
    ssh_dir = Path.home() / ".ssh"
    for pattern in ["id_ed25519", "id_rsa"]:
        key = ssh_dir / pattern
        if key.exists():
            return str(key)
    return None


def sign_data(key_path: str, data: bytes) -> str:
    """Sign data with SSH key using ssh-keygen -Y sign."""
    try:
        # Write data to temp file
        import tempfile
        with tempfile.NamedTemporaryFile(mode="wb", delete=False) as tf:
            tf.write(data)
            tf_path = tf.name

        # Sign with ssh-keygen
        result = subprocess.run(
            ["ssh-keygen", "-Y", "sign", "-f", key_path, "-n", "file", tf_path],
            capture_output=True,
            check=True,
        )
        os.unlink(tf_path)
        return result.stdout.strip()
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        if os.path.exists(tf_path):
            os.unlink(tf_path)
        raise RuntimeError(f"Signing failed: {e}")


def verify_signature(key_path: str, data: bytes, signature: str) -> bool:
    """Verify signature with ssh-keygen -Y verify."""
    try:
        import tempfile
        with tempfile.NamedTemporaryFile(mode="wb", delete=False) as tf:
            tf.write(data)
            tf_path = tf.name

        with tempfile.NamedTemporaryFile(mode="w", delete=False) as sf:
            sf.write(signature)
            sf_path = sf.name

        result = subprocess.run(
            ["ssh-keygen", "-Y", "verify", "-f", key_path, "-n", "file", "-s", sf_path, tf_path],
            capture_output=True,
        )
        os.unlink(tf_path)
        os.unlink(sf_path)
        return result.returncode == 0
    except (subprocess.CalledProcessError, FileNotFoundError):
        return False


def create_attestation(evidence_dir: Path, key_path: str | None = None) -> dict:
    """Create attestation for an evidence archive."""
    manifest = collect_manifest(evidence_dir)

    # Compute manifest hash
    manifest_json = json.dumps(manifest, sort_keys=True, separators=(",", ":")).encode()
    manifest_hash = hashlib.sha256(manifest_json).hexdigest()

    attestation = {
        "attestation_version": 1,
        "archive": evidence_dir.name,
        "created_at": datetime.utcnow().isoformat() + "Z",
        "git_commit": get_git_commit(),
        "git_dirty": get_git_dirty(),
        "manifest_hash": manifest_hash,
        "manifest": manifest,
    }

    # Sign if key available
    if key_path:
        attestation_json = json.dumps(attestation, sort_keys=True, separators=(",", ":")).encode()
        signature = sign_data(key_path, attestation_json)
        attestation["signature"] = signature
        attestation["signing_key"] = key_path
        attestation["signature_algorithm"] = "ssh-ed25519" if "ed25519" in key_path else "ssh-rsa"

    return attestation


def write_attestation(evidence_dir: Path, attestation: dict):
    """Write attestation to evidence directory."""
    attest_path = evidence_dir / "attestation.json"
    with open(attest_path, "w") as f:
        json.dump(attestation, f, indent=2, sort_keys=True)

    # Also write signature separately if present
    if "signature" in attestation:
        sig_path = evidence_dir / "attestation.sig"
        with open(sig_path, "w") as f:
            f.write(attestation["signature"])

    # Update assets.sha256 to include new files
    update_assets_sha256(evidence_dir)


def update_assets_sha256(evidence_dir: Path):
    """Regenerate assets.sha256 for the evidence directory."""
    assets_path = evidence_dir / "assets.sha256"
    with open(assets_path, "w") as f:
        for path in sorted(evidence_dir.rglob("*")):
            if path.is_file() and path.name not in ("assets.sha256",):
                rel = path.relative_to(evidence_dir)
                h = compute_file_hash(path)
                f.write(f"{h}  ./{rel}\n")


def verify_attestation(evidence_dir: Path, key_path: str | None = None) -> bool:
    """Verify attestation for an evidence archive."""
    attest_path = evidence_dir / "attestation.json"
    if not attest_path.exists():
        print(f"ERROR: No attestation.json found in {evidence_dir}")
        return False

    with open(attest_path) as f:
        attestation = json.load(f)

    # Verify manifest
    manifest = collect_manifest(evidence_dir)
    if manifest != attestation.get("manifest"):
        print("ERROR: Manifest mismatch")
        return False

    # Verify manifest hash
    manifest_json = json.dumps(manifest, sort_keys=True, separators=(",", ":")).encode()
    manifest_hash = hashlib.sha256(manifest_json).hexdigest()
    if manifest_hash != attestation.get("manifest_hash"):
        print("ERROR: Manifest hash mismatch")
        return False

    # Verify signature if present
    if "signature" in attestation and key_path:
        attestation_copy = dict(attestation)
        sig = attestation_copy.pop("signature")
        attestation_json = json.dumps(attestation_copy, sort_keys=True, separators=(",", ":")).encode()
        if not verify_signature(key_path, attestation_json, sig):
            print("ERROR: Signature verification failed")
            return False
        print("Signature verified OK")

    print(f"Attestation verified for {evidence_dir.name}")
    return True


def main():
    parser = argparse.ArgumentParser(description="Create or verify provenance attestation for evidence archives")
    parser.add_argument("evidence_dir", help="Path to evidence archive directory")
    parser.add_argument("--verify", action="store_true", help="Verify existing attestation")
    parser.add_argument("--key", help="Path to SSH signing key (default: auto-detect)")
    parser.add_argument("--no-sign", action="store_true", help="Create attestation without signature")

    args = parser.parse_args()

    evidence_dir = Path(args.evidence_dir).resolve()
    if not evidence_dir.exists() or not evidence_dir.is_dir():
        print(f"ERROR: Evidence directory not found: {evidence_dir}")
        return 1

    if args.verify:
        key = args.key or find_signing_key()
        if not key:
            print("ERROR: No signing key found for verification")
            return 1
        ok = verify_attestation(evidence_dir, key)
        return 0 if ok else 1

    # Create attestation
    key = None if args.no_sign else (args.key or find_signing_key())
    if key is None and not args.no_sign:
        print("WARNING: No SSH key found, creating unsigned attestation")

    try:
        attestation = create_attestation(evidence_dir, key)
        write_attestation(evidence_dir, attestation)
        print(f"Attestation created for {evidence_dir.name}")
        if "signature" in attestation:
            print(f"  Signed with: {attestation['signing_key']}")
        else:
            print("  Unsigned (no key available)")
        return 0
    except Exception as e:
        print(f"ERROR: {e}")
        return 1


if __name__ == "__main__":
    sys.exit(main())