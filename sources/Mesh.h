﻿class Mesh {
	PRIVATE struct ConstantBuffer {
		DirectX::XMMATRIX world;
		DirectX::XMFLOAT4 color;
	};

	PUBLIC DirectX::XMFLOAT3 position;
	PUBLIC DirectX::XMFLOAT3 angles;
	PUBLIC DirectX::XMFLOAT3 scale;
	PUBLIC DirectX::XMFLOAT4 color;
	PUBLIC std::vector<Vertex> vertexes;
	PUBLIC std::vector<int> indexes;
	PRIVATE ConstantBuffer cbuffer;
	PRIVATE ID3D11Buffer* vertexBuffer;
	PRIVATE ID3D11Buffer* indexBuffer;
	PRIVATE ID3D11Buffer* constantBuffer;

	PUBLIC Mesh() {
		position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		angles = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		color = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

		D3D11_BUFFER_DESC constantBufferDesc = {};
		constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = 0;
		App::GetGraphicsDevice().CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);
	}
	PUBLIC ~Mesh() {
	}
	PUBLIC void Apply() {
		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertexes.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
		vertexSubresourceData.pSysMem = &vertexes[0];
		App::GetGraphicsDevice().CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &vertexBuffer);

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		App::GetGraphicsContext().IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		App::GetGraphicsContext().IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3D11_BUFFER_DESC indexBufferDesc = {};
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(int) * indexes.size();
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA indexSubresourceData = {};
		indexSubresourceData.pSysMem = &indexes[0];
		App::GetGraphicsDevice().CreateBuffer(&indexBufferDesc, &indexSubresourceData, &indexBuffer);

		App::GetGraphicsContext().IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	}
	PUBLIC void Draw() {
		cbuffer.world = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) * DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(angles.x), DirectX::XMConvertToRadians(angles.y), DirectX::XMConvertToRadians(angles.z))* DirectX::XMMatrixTranslation(position.x, position.y, position.z);
		cbuffer.color = color;
		App::GetGraphicsContext().UpdateSubresource(constantBuffer, 0, nullptr, &cbuffer, 0, 0);
		App::GetGraphicsContext().VSSetConstantBuffers(0, 1, &constantBuffer);
		App::GetGraphicsContext().PSSetConstantBuffers(0, 1, &constantBuffer);

		App::GetGraphicsContext().DrawIndexed(indexes.size(), 0, 0);
	}
	PUBLIC static Mesh CreateQuad() {
		Mesh mesh;
		mesh.vertexes.push_back({ DirectX::XMFLOAT3(-0.5f, 0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f) });
		mesh.vertexes.push_back({ DirectX::XMFLOAT3(0.5f, 0.5f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f) });
		mesh.vertexes.push_back({ DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f) });
		mesh.vertexes.push_back({ DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f) });

		mesh.indexes.push_back(0);
		mesh.indexes.push_back(1);
		mesh.indexes.push_back(2);
		mesh.indexes.push_back(3);
		mesh.indexes.push_back(2);
		mesh.indexes.push_back(1);

		mesh.Apply();
		return mesh;
	}
};
