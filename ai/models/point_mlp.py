import torch
import torch.nn as nn


class PointMLP(nn.Module):
    """
    Baseline point-wise semantic segmentation network.

    Input:
        [B, N, 5]

    Output:
        [B, N, num_classes]
    """

    def __init__(
        self,
        num_classes=19,
        input_features=5,
    ):
        super().__init__()

        self.network = nn.Sequential(
            nn.Linear(input_features, 64),
            nn.BatchNorm1d(64),
            nn.ReLU(inplace=True),

            nn.Linear(64, 128),
            nn.BatchNorm1d(128),
            nn.ReLU(inplace=True),

            nn.Linear(128, 128),
            nn.BatchNorm1d(128),
            nn.ReLU(inplace=True),

            nn.Linear(128, 64),
            nn.BatchNorm1d(64),
            nn.ReLU(inplace=True),

            nn.Linear(64, num_classes),
        )

    def forward(self, x):

        batch_size, num_points, features = x.shape

        x = x.reshape(
            batch_size * num_points,
            features,
        )

        x = self.network(x)

        x = x.reshape(
            batch_size,
            num_points,
            -1,
        )

        return x